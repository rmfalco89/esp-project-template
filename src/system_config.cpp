#include "system_config.h"
#include "common/device_configuration.h"
#include "common/eeprom_utils.tpp"
#include "common/globals.h"

int SYSTEM_CONFIGURATION_EEPROM_ADDR;
SystemConfiguration *systemConfiguration = nullptr;

bool readConfigFromEeprom()
{
    DEBUG_PRINTLN(F("EEPROM: SPM configuration: read"));
    SystemConfiguration *eepromConfig = readDataFromEeprom<SystemConfiguration>(SYSTEM_CONFIGURATION_EEPROM_ADDR);
    if (eepromConfig != nullptr)
    {
        systemConfiguration = eepromConfig;
        DEBUG_PRINTLN(systemConfiguration->toStr());
        return true;
    }
    LOG_PRINTLN(F(">>WARNING: got invalid configuration info from EEPROM"));
    return false;
}

void saveConfigToEeprom()
{
    DEBUG_PRINTLN(F("EEPROM: configuration: write"));
    DEBUG_PRINTLN(systemConfiguration->toStr());
    if (systemConfiguration == nullptr)
        return;

    writeDataToEeprom<SystemConfiguration>(SYSTEM_CONFIGURATION_EEPROM_ADDR, systemConfiguration);
    DEBUG_PRINTLN(F("Done writing to EEPROM"));
}

void SystemConfiguration::initDefaultConfiguration()
{
    delete systemConfiguration;
    systemConfiguration = new SystemConfiguration('R');
}

void invalidateSystemConfigurationOnEeprom()
{
    DEBUG_PRINTLN(F("EEPROM: system configuration: invalidate"));
    invalidateEepromData<SystemConfiguration>(SYSTEM_CONFIGURATION_EEPROM_ADDR);
}
