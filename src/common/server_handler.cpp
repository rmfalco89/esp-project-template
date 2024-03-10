#include "server_handler.h"

#include <WebServer.h>
#include <HTTPClient.h>

WebServer *webServer;

void setupServer(bool config_mode)
{
    webServer = new WebServer(80);

    // Default routes
    webServer->on("/", HTTP_GET, routeHome);
    webServer->on("/reboot", HTTP_GET, rootReboot);
    webServer->on("/configure", HTTP_GET, routeConfigure);
    webServer->on("/saveConfiguration", HTTP_POST, routeSaveConfiguration);
    webServer->on("/invalidateConfig", HTTP_GET, routeInvaldateConfig);
    webServer->on("/checkForUpdates", HTTP_GET, routeCheckUpdate);
    // webServer->on("/uploadFirmware", HTTP_GET, routeUploadFirmware);
    // webServer->on("/firmwareUploadSave", HTTP_POST, routeUploadFirmwareSave);

    // Add routes here
    if (!config_mode) {
        // webServer->on("/routePath", HTTP_MODE, func);
    }

    // Start the server
    webServer->begin();
}


void loopServer()
{
    webServer->handleClient(); // Handle client requests
}
