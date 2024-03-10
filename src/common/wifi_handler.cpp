#include "wifi_handler.h"

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include "globals.h"
#include "device_configuration.h"

const char *ssid, *password, *hostname;
bool configMode_;

uint64_t lastCheckedMillis = 0;

bool connectWiFi(const char *ssid, const char *password, const char *hostname, bool configMode)
{
    bool connected = false;
    uint8_t numRetries = 5;

    if (configMode)
    {
        WiFi.mode(WIFI_AP);
        connected = WiFi.softAP(ssid, password);
    }
    else
    {
        while (!connected && numRetries-- >= 0)
        {
            WiFi.mode(WIFI_STA);
            WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);
            WiFi.begin(ssid, password);
            DEBUG_PRINT(F("Connecting to WiFi..."));
            uint64_t connectionAttemptBeginMillis = millis();

            while (millis() - connectionAttemptBeginMillis < wifiConnectionMaxMillis && !connected)
            {
                delay(500);
                DEBUG_PRINT(F("."));
                if (WiFi.status() == WL_CONNECTED)
                    connected = true;
                
            }
            if (!connected)
                DEBUG_PRINT(F("\nUnable to connect. Trying again."));
        }
        if (!connected)
        {
            DEBUG_PRINTLN(F("Connection to WiFi unsuccessful. Entering config mode"));
            saveJustRestartedToEeprom(true); // use the just restarted logic to enter in config mode
            ESP.restart();
        }
        DEBUG_PRINT(F("\nConnected to WiFi with IP address "));
        DEBUG_PRINTLN(WiFi.localIP());
    }

    // Initialize mDNS
    MDNS.end();
    delay(1000);
    if (!MDNS.begin(hostname))
    {
        Serial.println(F("Error setting up MDNS responder!"));
    }
    DEBUG_PRINTLN(F("mDNS responder started"));

    return true;
}

void setupWifi(bool configMode)
{
    DEBUG_PRINT(F("Setting up WiFi in "));
    DEBUG_PRINT(configMode ? F("AP") : F("STA"));
    DEBUG_PRINTLN(F(" mode."));
    configMode_ = configMode;
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

    WiFi.disconnect();
    delay(200);
    WiFi.softAPdisconnect(true);
    delay(200);
    WiFi.mode(WIFI_OFF); // Turn off to reset the Wi-Fi mode
    delay(3000);         // Short delay to allow Wi-Fi hardware to reset

    connectWiFi(ssid, password, hostname, configMode);
}

void loopWiFi()
{
    if (configMode_)
        return;

    // Check WiFi is still connected, otherwise reconnect
    if (millis() - lastCheckedMillis >= wifiConnectionStatusCheckMillis)
    {
        lastCheckedMillis = millis();
        if (WiFi.status() != WL_CONNECTED)
        {
            setupWifi(configMode_);
        }
    }
}
