#include "common/device_configuration.h"
#include "common/eeprom_utils.tpp"
#include "common/globals.h"

int JUST_RESTARTED_EEPROM_ADDR;
int DEVICE_CONFIGURATION_EEPROM_ADDR;

DeviceConfiguration *currentDeviceConfiguration = nullptr;

void DeviceConfiguration::printToSerial()
{
    String message = toStr();
    LOG_PRINTLN(message);
}

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
    LOG_PRINTLN(F(">>WARNING: got invalid DeviceConfiguration info from EEPROM"));
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

uint8_t readQuickRestartsFromEeprom()
{
    DEBUG_PRINTLN(F("EEPROM: just restarted: read...: "));
    QuickRestarts *eepromConfig = readDataFromEeprom<QuickRestarts>(JUST_RESTARTED_EEPROM_ADDR);
    if (eepromConfig == nullptr)
    {
        LOG_PRINTLN(F(">>WARNING: got invalid quickRestart info from EEPROM"));
        return 255;
    }
    DEBUG_PRINTLN(eepromConfig->consecutiveQuickRestartsCount == 0 ? "Not a quick restart!" : String(eepromConfig->consecutiveQuickRestartsCount));
    return eepromConfig->consecutiveQuickRestartsCount;
}

void saveQuickRestartsToEeprom(bool isQuickRestart)
{
    uint8_t restartsCount = !isQuickRestart ? 0 : readQuickRestartsFromEeprom() + 1;

    DEBUG_PRINTLN("EEPROM: just restarted: write: count: " + String(restartsCount));
    QuickRestarts qr(restartsCount);
    writeDataToEeprom<QuickRestarts>(JUST_RESTARTED_EEPROM_ADDR, &qr);
    DEBUG_PRINTLN(F(" ..done"));
}
