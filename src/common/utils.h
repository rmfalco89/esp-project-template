#ifndef UTILS_H
#define UTILS_H

#include "Arduino.h"

String stringMask(const String &str, char mask);
String getWifiStrength();
String millisToTimeStr(uint64_t);
#endif // UTILS_H