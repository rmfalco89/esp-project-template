#ifndef SERVER_HANDLES_H
#define SERVER_HANDLES_H

#include "ESPAsyncWebServer.h"

void routeHomeComplete(AsyncWebServerRequest *request);
void addServerHandles();

#endif // SERVER_HANDLES_H