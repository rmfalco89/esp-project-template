#include "Arduino.h"

#include "server_handler.h"
#include "globals.h"

#include "device_configuration.h"
#include "wifi_handler.h"

// Routes go here
void routeHome()
{
    DEBUG_PRINTLN("routeHome");

    String currentConfigStr = "\n\n---\nSoftware version: " + String(SW_VERSION);

    if (currentDeviceConfiguration != nullptr)
        currentConfigStr += currentDeviceConfiguration->toStr();
    else
        currentConfigStr += F("No valid configuration was found.");

    currentConfigStr += "\n\n---\nJust restarted: " + String(justRestarted ? "yes" : "no");

    webServer->send(200, "text/plain", currentConfigStr);
}
void rootReboot()
{
    DEBUG_PRINTLN("rootReboot");

    webServer->send(200, "text/plain", F("Rebooting now."));
    delay(3000);
    ESP.restart();
}

void routeInvaldateConfig()
{
    DEBUG_PRINTLN("routeInvaldateConfig");

    invalidateDeviceConfigurationOnEeprom();
    webServer->send(200, "text/plain", F("Configuration voided. Configure at /configure. Rebooting now."));
    ESP.restart();
}

void routeCheckUpdate()
{
    DEBUG_PRINTLN("routeCheckUpdate");

    webServer->send(200, "text/html", F("Checking for new firmware on github. This might take a few seconds..."));
    updater->upgradeSoftware();
}

String formatDeviceConfigurationHtmlTemplate()
{
    String ssid_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->ssid;
    String password_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->password;
    String hostname_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->hostname;
    String deviceName_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->deviceName;
    String githubAuthToken_str = currentDeviceConfiguration == nullptr ? "" : currentDeviceConfiguration->githubAuthToken;

    return String("<!DOCTYPE html> \
    <html><head><meta charset=\"utf-8\"><title>Configuration</title></head><body> \
    <form method=\"post\" action=\"/saveConfiguration\"> \
<label for=\"ssid\">WiFi SSID:</label>\
                        <input type =\"text\" id=\"ssid\" name=\"ssid\" value=\"" +
                  ssid_str + "\">\
<br><br>\
<label for=\"password\">WiFi Password:</label> \
<input type=\"password\" id=\"password\" name=\"password\" value=\"" +
                  password_str + "\">\
<br><br>\
<label for=\"hostname\">Hostname</label>\
<input type=\"text\" id=\"hostname\" name=\"hostname\" value=\"" +
                  hostname_str + "\">\
<br><br>\
<label for=\"device_name\">Device name</label>\
<input type=\"text\" id=\"device_name\" name=\"device_name\" value=\"" +
                  deviceName_str + "\"> \
<br><br>\
<label for=\"auth_token\">Github Auth Token</label>\
<input type=\"text\" id=\"auth_token\" name=\"auth_token\" value=\"" +
                  githubAuthToken_str + "\">\
<br><br>\
\
<input type=\"submit\" value=\"Save\"></form></body></html>");
}

void routeConfigure()
{
    DEBUG_PRINTLN("routeConfigure");
    webServer->send(200, "text/html", formatDeviceConfigurationHtmlTemplate());
}

void routeSaveConfiguration()
{
    DEBUG_PRINTLN("routeSaveConfiguration");

    String ssid = webServer->arg("ssid");
    String password = webServer->arg("password");
    String hostName = webServer->arg("hostname");
    String deviceName = webServer->arg("device_name");
    String authToken = webServer->arg("auth_token");

    if (hostName == "")
        hostName = configModeHostname;

    DeviceConfiguration *newConfig = new DeviceConfiguration(ssid.c_str(), password.c_str(), hostName.c_str(), deviceName.c_str(), authToken.c_str());

    // Send a response to the client
    webServer->send(200, "text/plain", F("Configuration received. Will attempt connection to WiFi with provided credentials. Will save configuration if successful."));
    delay(250);

    currentDeviceConfiguration = newConfig;
    saveDeviceConfigurationToEeprom();
    setupWifi(false);
    Serial.println(F("Configuration accepted. Rebooting."));
    ESP.restart(); // Note: will not reach this point if connection to Wifi is unsuccessful
}