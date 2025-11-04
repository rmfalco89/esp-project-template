#ifdef ESP32
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#endif


#include "globals.h"
#include "common/utils.h"
#include "serverHandles.h"

void routeHomeComplete(AsyncWebServerRequest *request)
{
    DEBUG_PRINTLN("routeHome");

    // Software Version
    String currentConfigStr = "\n\n---\nSoftware version: " + String(SW_VERSION);

    // Quick restarts
    currentConfigStr += "\n\n---\nQuick restarts count: " + String(quickRestartsCount);

    // Wifi signal strength
    String wifiStrength = getWifiStrength();
    currentConfigStr += "\n\n---\nWifi Signal strength: " + wifiStrength;
    currentConfigStr += "\nHostname: " + String(currentDeviceConfiguration->hostname);

    // Current common configuration
    // currentConfigStr += "\n\n---\nDevice configuration:\n";
    // if (currentDeviceConfiguration != nullptr)
    //     currentConfigStr += currentDeviceConfiguration->toStr();
    // else
    //     currentConfigStr += F("\tNo valid configuration was found.");

    // Routes description
    if (!routeDescriptions.empty())
    {
        currentConfigStr += "\n\n---\nAvailable services:\n";
        // Iterate over the map and print the key-value pairs
        for (const auto &pair : routeDescriptions)
        {
            currentConfigStr += pair.first;
            currentConfigStr += !pair.second.isEmpty() ? (": " + pair.second) : "";
            currentConfigStr += "\n";
        }
    }

    // Components data
    // <components_data>

    request->send(200, "text/plain", currentConfigStr);
}

void addServerHandles()
{
    webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { routeHomeComplete(request); });
    routeDescriptions["/"] = "";

    webServer->on("/configure", HTTP_GET, [](AsyncWebServerRequest *request)
                  { routeConfigureBoard(request); });
    routeDescriptions["/configure"] = "Configure sump pump manager settings";
}

String formatConfigurationHtmlTemplate()
{
    String myConfig = String(systemConfiguration->myConfig);

    return String("<!DOCTYPE html><html><head><meta charset='utf-8'><title>System Configuration</title></head><body>\
<form method='post' action='/saveConfig'>\
<label for='myConfig'>My config char:</label>\
<input type='text' id='myConfig' name='myConfig' value='" +
                  myConfig + "'><br><br>\
<input type='submit' value='Save'></form></body></html>");
}

void routeConfigureBoard(AsyncWebServerRequest *request)
{
    DEBUG_PRINTLN("routeConfigureBoard")
    request->send(200, "text/html", formatConfigurationHtmlTemplate());
}
