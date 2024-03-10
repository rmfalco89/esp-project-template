#include <Arduino.h>

#include "common/common_main.h"
#include "common/globals.h"

void setup()
{
  common_setup();

  webServer->on("/dynamicRoute", HTTP_GET, []()
                { webServer->send(200, "text/plain", "This is a dynamically added route."); });
}

void loop()
{
  common_loop();
  // Your loop code here
  delay(1000);
}