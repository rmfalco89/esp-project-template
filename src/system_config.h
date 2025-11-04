#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include "Arduino.h"

// Data Structure Alignment
#pragma pack(push, 1)

struct SystemConfiguration
{
    char myConfig;

    SystemConfiguration() {}

    SystemConfiguration(char myConfig_): myConfig(myConfig_) {}

    String toStr() const
    {
        String text = "###:\n";
        text += "\nMy config: " + String(myConfig);
        return text;
    }

    static void initDefaultConfiguration();
};

bool readConfigFromEeprom();
void saveConfigToEeprom();
void invalidateSystemConfigurationOnEeprom();

#pragma pack(pop)

#endif // SENSOR_CONFIG_H
