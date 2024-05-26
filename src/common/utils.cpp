#include "utils.h"

#include <Ticker.h>

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

String stringMask(const String &str, char mask)
{
    String masked;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        masked += mask;
    }
    return masked;
}

Ticker restartTimer;
void scheduleRestart(uint8_t seconds = 3)
{
    static bool restartScheduled = false;
    if (!restartScheduled)
    {
        restartScheduled = true;
        restartTimer.once(seconds, []()
                          { ESP.restart(); });
    }
}

String getWifiStrength()
{
    long rssi = WiFi.RSSI();
    String signalStrength;

    if (rssi > -50)
    {
        signalStrength = "Excellent";
    }
    else if (rssi > -60)
    {
        signalStrength = "Good";
    }
    else if (rssi > -70)
    {
        signalStrength = "Medium";
    }
    else if (rssi > -80)
    {
        signalStrength = "Low";
    }
    else
    {
        signalStrength = "Very Low";
    }

    return signalStrength;
}

String millisToTimeStr(uint64_t millisValue)
{
    unsigned long seconds = millisValue / 1000; // Convert milliseconds to seconds
    uint32_t days = seconds / 86400;            // Calculate days
    seconds %= 86400;                           // Remaining seconds after calculating days
    uint16_t hours = seconds / 3600;            // Calculate hours
    seconds %= 3600;                            // Remaining seconds after calculating hours
    uint8_t minutes = seconds / 60;             // Calculate minutes
    seconds %= 60;                              // Remaining seconds are full seconds

    String timeStr = "";
    if (days)
        timeStr += String(days) + "days ";
    if (hours)
        timeStr += String(hours) + "h:";
    if (minutes)
        timeStr += String(minutes) + "m:";

    timeStr += String(seconds) + "s";
    return timeStr;
}