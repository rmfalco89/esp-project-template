#include "common/memory_stats.h"
#include "common/globals.h"

MemoryStats ramStats;
uint64_t memStatsLastUpdatedMillis = 0;

void updateMemoryStats()
{
    if (millis() - memStatsLastUpdatedMillis < ramStatsUpdateIntervalMillis)
        return;
#ifdef ESP32
    uint32_t freeHeap = esp_get_free_heap_size();
#elif defined(ESP8266)
    uint32_t freeHeap = ESP.getFreeHeap();
#endif

    ramStats.addSample(freeHeap);
}
