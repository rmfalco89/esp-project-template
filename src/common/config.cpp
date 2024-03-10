#include "globals.h"

// Firmware
const char *SW_VERSION = "0.1.2";
const char *BINARY_NAME = "esp32devkitc.bin";

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

// GitHub
const char *releaseRepo = "rmfalco89/<myproj>";
const char *GITHUB_TOKEN = "your_github_token"; // Update with your GitHub token
