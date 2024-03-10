#include "globals.h"

// Firmware
const char *SW_VERSION = "0.1.0";
const char *BINARY_NAME = "esp32devkitc.bin";

// GitHub
const char *releaseRepo = "rmfalco89/esp32-project-template";

// Watchdog
const int watchdogTimeout_s = 30;

// Quick Restart && Config Mode
const uint16_t quickRestarMaxDurationMillis = 30 * 1000;   // 30s
const uint32_t configModeCheckEveryMillis = 5 * 60 * 1000; // 5m

// Wifi
const char *configModeSsid = "ArduinoNet";
const char *configModeHostname = "arduino";
const uint32_t wifiConnectionStatusCheckMillis = 20 * 60 * 1000; // 20s
const uint16_t wifiConnectionMaxMillis = 20 * 1000;              // 30s
const IPAddress dns(8, 8, 8, 8);                                 // Google's DNS

