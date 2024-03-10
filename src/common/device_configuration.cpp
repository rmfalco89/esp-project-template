#include "device_configuration.h"
#include "eeprom_utils.tpp"

DeviceConfiguration *currentDeviceConfiguration = nullptr;

bool readDeviceConfigurationFromEeprom()
{
    DEBUG_PRINTLN(F("EEPROM: device configuration: read"));
    DeviceConfiguration *eepromConfig = readDataFromEeprom<DeviceConfiguration>(DEVICE_CONFIGURATION_EEPROM_ADDR);
    if (eepromConfig != nullptr)
    {
        currentDeviceConfiguration = eepromConfig;
        DEBUG_PRINTLN(currentDeviceConfiguration->toStr());
        return true;
    }
    Serial.println(F(">>WARNING: got invalid DeviceConfiguration info from EEPROM"));
    return false;
}

void saveDeviceConfigurationToEeprom()
{
    DEBUG_PRINTLN(F("EEPROM: device configuration: write"));
    DEBUG_PRINTLN(currentDeviceConfiguration->toStr());
    if (currentDeviceConfiguration == nullptr)
        return;

    writeDataToEeprom<DeviceConfiguration>(DEVICE_CONFIGURATION_EEPROM_ADDR, currentDeviceConfiguration);
    DEBUG_PRINTLN(F("Done writing to EEPROM"));
}

void invalidateDeviceConfigurationOnEeprom()
{
    DEBUG_PRINTLN(F("EEPROM: device configuration: invalidate"));
    invalidateEepromData<DeviceConfiguration>(DEVICE_CONFIGURATION_EEPROM_ADDR);
}

bool readJustRestartedFromEeprom()
{
    DEBUG_PRINT(F("EEPROM: just restarted: read...: "));
    JustRestarted *eepromConfig = readDataFromEeprom<JustRestarted>(JUST_RESTARTED_EEPROM_ADDR);
    if (eepromConfig == nullptr)
    {
        Serial.println(F(">>WARNING: got invalid quickRestart info from EEPROM"));
        return false;
    }
    DEBUG_PRINTLN(String(eepromConfig->justRestarted ? "true" : "false"));
    return eepromConfig->justRestarted;
}

void saveJustRestartedToEeprom(bool is_quick_restart)
{
    DEBUG_PRINT(String("EEPROM: just restarted: write: ") + String(is_quick_restart ? "true" : "false"));
    JustRestarted qr(is_quick_restart);
    writeDataToEeprom<JustRestarted>(JUST_RESTARTED_EEPROM_ADDR, &qr);
    DEBUG_PRINTLN(F(" ..done"));
}

// #include "device_configuration.h"
// #include "eeprom_esp32.tpp"
// #include "globals.h"

// DeviceConfiguration *currentDeviceConfiguration = nullptr;

// bool readDeviceConfigurationFromEeprom()
// {
//     DEBUG_PRINTLN(F("Reading device configuration from eeprom"));

//     DeviceConfiguration *config = readDataFromPreferences<DeviceConfiguration>("deviceConfig");
//     if (config != nullptr)
//     {
//         currentDeviceConfiguration = config;
//         return true;
//     }
//     Serial.println(F(">>WARNING: got invalid DeviceConfiguration info from EEPROM"));
//     return false;
// }

// void saveDeviceConfigurationToEeprom()
// {
//     DEBUG_PRINTLN(F("Saving device configuration to eeprom"));
//     if (currentDeviceConfiguration == nullptr)
//         return;
//     writeDataToPreferences<DeviceConfiguration>("deviceConfig", currentDeviceConfiguration);
// }

// void invalidateDeviceConfigurationOnEeprom()
// {
//     DEBUG_PRINTLN(F("Invalidating EEPROM device configuration"));

//     invalidatePreferencesData<DeviceConfiguration>("deviceConfig");
// }

// bool getJustRestartedInfoFromEeprom()
// {
//     DEBUG_PRINTLN(F("Reading just restarted from eeprom"));

//     JustRestarted *config = readDataFromPreferences<JustRestarted>("justRestarted");
//     if (config != nullptr)
//     {
//         bool justRestarted = config->justRestarted;
//         delete config; // Clean up the allocated memory
//         return justRestarted;
//     }
//     Serial.println(F(">>WARNING: got invalid justRestarted info from EEPROM"));
//     return false;
// }

// void saveJustRestartedToEeprom(bool is_quick_restart)
// {
//     DEBUG_PRINTLN(F("Saving just restarted to eeprom"));

//     JustRestarted qr(is_quick_restart);
//     writeDataToPreferences<JustRestarted>("justRestarted", &qr);
// }
