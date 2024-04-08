#include <HTTPClient.h>

#include "common/globals.h"
#include "globals.h"
#include "common/utils.h"

void routeHomeComplete(AsyncWebServerRequest *request)
{
    DEBUG_PRINTLN("routeHome");

    // Software Version
    String currentConfigStr = "\n\n---\nSoftware version: " + String(SW_VERSION);

    // Just restarted
    currentConfigStr += "\n\n---\nJust restarted: " + String(justRestarted ? "yes" : "no");

    // Wifi signal strength
    String wifiStrength = getWifiStrength();
    currentConfigStr += "\n\n---\nWifi Signal strength: " + wifiStrength;

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
}
