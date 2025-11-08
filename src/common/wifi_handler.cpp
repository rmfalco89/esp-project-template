#include "wifi_handler.h"

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include "common/globals.h"
#include "device_configuration.h"

const char *ssid, *password, *hostname;

uint64_t lastCheckedMillis = 0;

bool connectWiFi(const char *ssid, const char *password, const char *hostname)
{
    bool connected = false;
    uint8_t numRetries = 5;
    IPAddress ipAddress = IPAddress((uint32_t)0);

    // Disable WiFi persistence to prevent flash wear and unexpected reconnections
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);

#ifdef ESP8266
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
#elif defined(ESP32)
    WiFi.setSleep(false);  // Disable WiFi sleep to reduce serial corruption
    esp_wifi_set_ps(WIFI_PS_NONE); // Explicitly disable power save for best performance
#endif

    if (configMode)
    {
        WiFi.mode(WIFI_AP);
        connected = WiFi.softAP(ssid, password);
        ipAddress = WiFi.softAPIP();
    }
    else
    {
        while (!connected && numRetries-- > 0)
        {
            WiFi.mode(WIFI_STA);
            // Uncomment to set dns
            // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);
            WiFi.begin(ssid, password);
            DEBUG_PRINT(F("Connecting to WiFi..."));
            uint64_t connectionAttemptBeginMillis = millis();

            while (millis() - connectionAttemptBeginMillis < wifiConnectionMaxMillis && !connected)
            {
                delay(500);
                DEBUG_PRINT(F("."));
                if (WiFi.status() == WL_CONNECTED)
                {
                    connected = true;
                    ipAddress = WiFi.localIP();
                }
            }
            if (!connected)
                DEBUG_PRINTLN(F("\nUnable to connect. Trying again."));
        }
        if (!connected)
        {
            DEBUG_PRINTLN(F("Connection to WiFi unsuccessful."));
            return false;  // Let the caller handle config mode transition
        }
        DEBUG_PRINTLN("\nConnected to WiFi with IP address " + ipAddress.toString());
    }

    // Initialize mDNS
    MDNS.end();
#ifdef ESP8266
    delay(2000); // ESP8266 needs longer delay for mDNS cleanup
#else
    delay(1000);
#endif

    if (!MDNS.begin(hostname))
    {
        LOG_PRINTLN(F("Error setting up mDNS responder!"));
        // Retry once
        delay(1000);
        if (!MDNS.begin(hostname))
        {
            LOG_PRINTLN(F("mDNS failed after retry!"));
        }
        else
        {
            DEBUG_PRINTLN("mDNS responder started with hostname " + String(hostname) + " after retry");
            // Advertise HTTP service
            MDNS.addService("http", "tcp", 80);
        }
    }
    else
    {
        DEBUG_PRINTLN("mDNS responder started with hostname " + String(hostname));
        // Advertise HTTP service for better discoverability
        MDNS.addService("http", "tcp", 80);
    }

    return true;
}

bool setupWifi()
{
    LOG_PRINT(F("Setting up WiFi in "));
    LOG_PRINT(configMode ? F("AP") : F("STA"));
    LOG_PRINTLN(F(" mode."));
    if (configMode)
    {
        ssid = configModeSsid;
        password = "";
        hostname = configModeHostname;
    }
    else
    {
        ssid = currentDeviceConfiguration->ssid;
        password = currentDeviceConfiguration->password;
        hostname = currentDeviceConfiguration->hostname;
    }

    // Disconnect cleanly based on current mode
    wifi_mode_t currentMode = WiFi.getMode();

    if (currentMode == WIFI_STA || currentMode == WIFI_AP_STA)
    {
        WiFi.disconnect(true, true); // disconnect and erase AP info
#ifdef ESP8266
        delay(500); // ESP8266 needs more time
#else
        delay(100);
#endif
    }

    if (currentMode == WIFI_AP || currentMode == WIFI_AP_STA)
    {
        WiFi.softAPdisconnect(true);
#ifdef ESP8266
        delay(500); // ESP8266 needs more time
#else
        delay(100);
#endif
    }

    WiFi.mode(WIFI_OFF);

#ifdef ESP8266
    delay(1000); // ESP8266 needs longer to fully power down WiFi
#else
    delay(500);  // ESP32 can switch faster
#endif

    // Verify mode changed
    if (WiFi.getMode() != WIFI_OFF)
    {
        LOG_PRINTLN(F("Warning: WiFi failed to turn off completely"));
    }

    return connectWiFi(ssid, password, hostname);
}

void loopWiFi()
{
#ifdef ESP8266
    MDNS.update();
#endif

    if (configMode)
        return;

    // Make sure WiFi is connected, reconnect if necessary
    if (millis() - lastCheckedMillis >= wifiConnectionStatusCheckMillis)
    {
        lastCheckedMillis = millis();
        WiFiClient client;
        const char *host = "www.google.com";
        const int port = 80; // HTTP port

        if (WiFi.status() != WL_CONNECTED || !client.connect(host, port))
        {
            LOG_PRINTLN("WiFi disconnected. Attempting WiFi setup");
            setupWifi();
        }
        client.stop(); // Explicitly close connection
    }
}

String getIPAddress()
{
    if (WiFi.getMode() == WIFI_STA)
        return WiFi.localIP().toString();
    else if (WiFi.getMode() == WIFI_AP)
        return WiFi.softAPIP().toString();
    return "";
}
