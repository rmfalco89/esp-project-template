#ifndef SERVER_HANDLES_H
#define SERVER_HANDLES_H

#include "ESPAsyncWebServer.h"

void addServerHandles();

void routeHomeComplete(AsyncWebServerRequest *request);
void routeConfigureBoard(AsyncWebServerRequest *request);

#endif // SERVER_HANDLES_H