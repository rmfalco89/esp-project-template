#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <Arduino.h>

bool setupWifi();
void loopWiFi();
String getIPAddress();

#endif