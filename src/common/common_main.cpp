#include <Arduino.h>
#include <esp_task_wdt.h>
#include <esp_sleep.h>

#include "common/device_configuration.h"
#include "common/eeprom_utils.tpp"
#include "common/globals.h"
#include "common/ota_handler.h"
#include "common/server_handler.h"
#include "common/wifi_handler.h"

bool configMode = false;
bool justRestarted = true;
uint64_t configModeLastCheckMillis = 0;
ESP32_GithubOtaUpdate *updater = nullptr;

void common_setup()
{
    // Enable the task watchdog with a timeout of 30 seconds
    esp_task_wdt_init(watchdogTimeout_s, true); // true to cause a panic (reset) when the timeout is reached
    esp_task_wdt_add(NULL);                     // Passing NULL adds the current task (loop task for Arduino)

    Serial.begin(115200);
    LOG_PRINTLN(F("==============\n== Welcome! ==\n=============="));

    // init EEPROM addresses
    JUST_RESTARTED_EEPROM_ADDR = 0;
    DEVICE_CONFIGURATION_EEPROM_ADDR = nextEepromSlot<JustRestarted>(JUST_RESTARTED_EEPROM_ADDR);

    // Check whether it's a quick restart or the device config is not valid
    readDeviceConfigurationFromEeprom();
    if (readJustRestartedFromEeprom() || !currentDeviceConfiguration)
        configMode = true;
    else
        configMode = false;

    // Quick Restart
    saveJustRestartedToEeprom(true);

    // Wifi setup
    setupWifi(configMode);

    // Server setuo
    setupServer(configMode);

    // OTA Updater
    updater = new ESP32_GithubOtaUpdate(SW_VERSION, BINARY_NAME, releaseRepo, currentDeviceConfiguration->githubAuthToken);
    updater->registerFirmwareUploadRoutes(webServer, &routeDescriptions);
    if (!configMode)
        updater->upgradeSoftware(); // Check and perform upgrade on startup

    LOG_PRINTLN("SW_VERSION: " + String(SW_VERSION));
    LOG_PRINTLN("Common setup complete");
}

void common_loop()
{
    // Housekeeping //

    // - watchdog
    esp_task_wdt_reset();

    // - check if just restarted
    if (justRestarted && millis() > quickRestarMaxDurationMillis)
    {
        saveJustRestartedToEeprom(false);
        justRestarted = false;
    }
    // - check if in config mode but a valid configuration is found.
    // This covers the case where connection to WiFI was temporarily unsuccessful but the configuration is valid
    if (configMode && millis() - configModeLastCheckMillis > configModeCheckEveryMillis)
    {
        configModeLastCheckMillis = millis();
        if (readDeviceConfigurationFromEeprom()) {
            LOG_PRINTLN("Got valid configuration and connected to wifi. Restarting.");
            delay(50);
            ESP.restart();
        }
    }

    // - wifi and server
    loopWiFi();
    loopServer();

    // - ota software updates
    if (!configMode)
        updater->checkForSoftwareUpdate();

    // End of Housekeeping //
}