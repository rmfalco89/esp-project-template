#ifndef GLOBALS_H
#define GLOBALS_H

#include <WebServer.h>

#include "device_configuration.h"
#include "ota_handler.h"

extern bool configMode;
extern bool justRestarted;

extern const char *SW_VERSION;
extern const char *BINARY_NAME;
extern const char *releaseRepo;

extern DeviceConfiguration *currentDeviceConfiguration;
extern ESP32_GithubOtaUpdate *updater;
extern WebServer *webServer;

// Firmware
extern const char *SW_VERSION;
extern const char *BINARY_NAME;

// Watchdog
extern const int watchdogTimeout_s;

// Just Restarted
extern const uint16_t quickRestarMaxDurationMillis;
extern const uint32_t configModeCheckEveryMillis;

// Wifi
extern const char *configModeSsid;
extern const char *configModeHostname;
extern const uint32_t wifiConnectionStatusCheckMillis;
extern const uint16_t wifiConnectionMaxMillis;
extern const IPAddress dns;

// GitHub
extern const char *releaseRepo;
extern const char *GITHUB_TOKEN;


// Uncomment the following line to enable debug output.
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(str)   \
    {                      \
        Serial.print(str); \
    }
#define DEBUG_PRINTLN(str)   \
    {                        \
        Serial.println(str); \
    }
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)
#endif

#endif // GLOBALS_H