#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <Arduino.h>
#include "globals.h"

#ifdef ESP8266
ADC_MODE(ADC_VCC); // Required for ESP8266, must be before setup()
#endif

// Store last voltage to detect sudden drops
static float lastVoltage = 3.3;       // Initialize to a safe value
static unsigned long lastLogTime = 0; // Last time a warning was logged

/**
 * Reads VCC voltage.
 * - On ESP8266, uses built-in VCC measurement.
 * - On ESP32, reads from ADC pin 35 (must be wired to VCC).
 */
inline float readVCC()
{
#ifdef ESP8266
    return ESP.getVcc() / 1000.0; // Convert mV to V
#elif defined(ESP32)
    return (analogRead(35) * 3.3) / 4095.0; // Adjust if using a voltage divider
#else
    return -1.0; // Unsupported board
#endif
}

/**
 * Logs the VCC voltage, warns if it's too low, and detects sudden power drops.
 */
inline void logVCC()
{
    float voltage = readVCC();
    unsigned long currentTime = millis();

    // Only print warnings at most once every 5 seconds (5000 ms)
    if (currentTime - lastLogTime >= 5000)
    {
        if (voltage > 0 && voltage < 3.2)
        {
            LOG_PRINT(F("VCC Voltage: "));
            LOG_PRINTLN(String(voltage) + "V");

            LOG_PRINTLN(F("⚠️ WARNING: Low voltage detected! Potential power issue."));
        }

        // Detect sudden drops (e.g., 0.2V drop in 1 cycle)
        if (lastVoltage - voltage > 0.2)
        {
            LOG_PRINT(F("VCC Voltage: "));
            LOG_PRINTLN(String(voltage) + "V");

            LOG_PRINTLN(F("🚨 CRITICAL POWER DROP DETECTED! Check power supply."));
        }

        lastLogTime = currentTime;
    }

    lastVoltage = voltage; // Update last voltage for next check
}

/**
 * Checks the cause of the last reset and logs it (only needed at startup).
 */
inline void checkResetCause()
{
#ifdef ESP8266
    rst_info *resetInfo = ESP.getResetInfoPtr();
    LOG_PRINT(F("ESP8266 Reset reason: "));
    LOG_PRINTLN(resetInfo->reason);
#elif defined(ESP32)
    esp_reset_reason_t reason = esp_reset_reason();
    LOG_PRINT(F("ESP32 Reset reason: "));

    switch (reason)
    {
    case ESP_RST_POWERON:
        LOG_PRINTLN(F("Power-on Reset"));
        break;
    case ESP_RST_BROWNOUT:
        LOG_PRINTLN(F("⚠️ Brownout Reset: Possible power issue!"));
        break;
    case ESP_RST_PANIC:
        LOG_PRINTLN(F("⚠️ Crash/Panic Reset"));
        break;
    case ESP_RST_SW:
        LOG_PRINTLN(F("Software Reset"));
        break;
    default:
        LOG_PRINT(F("Unknown Reset Cause: "));
        LOG_PRINTLN(String(reason));
        break;
    }
#endif
}

#endif // POWER_MONITOR_H
