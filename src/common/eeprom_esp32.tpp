// #ifndef EEPROM_ESP32_H
// #define EEPROM_ESP32_H

// #include <Preferences.h>

// #include "globals.h"

// Preferences preferences;

// using checksum_type = uint32_t;

// template <typename T>
// T *readDataFromPreferences(const char *key) {
//     T *data = new T();
//     if (preferences.begin("storage", true)) {
//         size_t readSize = preferences.getBytes(key, data, sizeof(T));
//         preferences.end();

//         if (readSize == sizeof(T)) {
//             return data;
//         }
//     }
//     delete data; // Free memory if read failed
//     return nullptr;
// }

// template <typename T>
// void writeDataToPreferences(const char *key, T *data) {
//     if (preferences.begin("storage", false)) {
//         preferences.putBytes(key, data, sizeof(T));
//         preferences.end();
//     }
// }

// template <typename T>
// void invalidatePreferencesData(const char *key) {
//     if (preferences.begin("storage", false)) {
//         preferences.remove(key);
//         preferences.end();
//     }
// }

// #endif // EEPROM_ESP32_H
