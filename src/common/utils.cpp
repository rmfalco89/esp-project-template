#include "utils.h"

#include <Ticker.h>
#include <WiFi.h>

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
void scheduleRestart(uint8_t seconds=3)
{
    static bool restartScheduled = false;
    if (!restartScheduled)
    {
        restartScheduled = true;
        restartTimer.once(seconds, [](){ ESP.restart(); });
    }
}


String getWifiStrength() {
    long rssi = WiFi.RSSI();
    String signalStrength;

    if (rssi > -50) {
        signalStrength = "Excellent";
    } else if (rssi > -60) {
        signalStrength = "Good";
    } else if (rssi > -70) {
        signalStrength = "Medium";
    } else if (rssi > -80) {
        signalStrength = "Low";
    } else {
        signalStrength = "Very Low";
    }

    return signalStrength;
}