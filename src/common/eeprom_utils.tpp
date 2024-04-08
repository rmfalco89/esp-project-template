#ifndef EEPROM_UTILS_TPP
#define EEPROM_UTILS_TPP

#include <EEPROM.h>
#include <Arduino.h>

#include "common/globals.h"

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 512
#endif

using checksum_type = uint32_t;

template <typename T>
T *readDataFromEeprom(const int eepromAddress)
{
    DEBUG_PRINTLN(String("Reading from EEPROM address ") + String(eepromAddress));
    EEPROM.begin(EEPROM_SIZE);

    // read checksum first, then data
    checksum_type expectedChecksum;
    EEPROM.get(eepromAddress, expectedChecksum);

    T *data = new T();
    EEPROM.get(eepromAddress + sizeof(checksum_type), *data);
    EEPROM.end();

    checksum_type checksum = calculateChecksum(data);

    if (checksum == expectedChecksum)
        return data;
    else
        return nullptr;
}

template <typename T>
void writeDataToEeprom(int eepromAddress, T *data)
{
    DEBUG_PRINTLN(String("Writing to EEPROM address ") + String(eepromAddress));
    EEPROM.begin(EEPROM_SIZE);

    checksum_type checksum = calculateChecksum(data);
    EEPROM.put(eepromAddress, checksum);
    EEPROM.put(eepromAddress + sizeof(checksum_type), *data);

    EEPROM.commit();
    EEPROM.end();
}

template <typename T>
checksum_type calculateChecksum(const T *data)
{
    checksum_type checksum = 0;
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data);
    for (size_t i = 0; i < sizeof(T); i++)
        checksum += *ptr++;

    return checksum;
}

template <typename T>
void invalidateEepromData(const int eepromAddress)
{
    EEPROM.begin(EEPROM_SIZE);

    checksum_type empty = 0;
    EEPROM.put(eepromAddress, empty);
    EEPROM.commit();
    EEPROM.end();
}

template <typename T>
int nextEepromSlot(int previousSlotStartAddress)
{
    // checksum + size of previous struct:
    int address = sizeof(T) + sizeof(checksum_type);
    // add offset:
    address += previousSlotStartAddress;
    return address;
}

#endif