#include "time_sync.h"
#include "config_manager.h"
#include "log_manager.h"
#include "time_manager.h"

struct tm cachedTime = {0};  // Глобальное кэшированное время
unsigned long lastMillis = 0;  // Время последнего обновления миллисекунд
bool timeSyncStarted = false;  // Флаг запущенной синхронизации

void syncTimeTask(void *parameter) {
    while (true) {
        if (WiFi.status() == WL_CONNECTED) {
            if (!updateTimeZone()) updateTimeZone();
            // logEvent("Setting time config: gmtOffset_sec=" + String(gmtOffset_sec) + ", daylightOffset_sec=" + String(daylightOffset_sec));
            // configTime(gmtOffset_sec, daylightOffset_sec, String(config["NTP_SERVER"]).c_str());
            configTime(gmtOffset_sec, 0, String(config["NTP_SERVER"]).c_str());
            struct tm timeInfo;
            if (getLocalTime(&timeInfo)) {
                cachedTime = timeInfo; // Обновляем кэш
                lastMillis = millis(); // Фиксируем момент синхронизации
                // logEvent("Time synced: " + String(asctime(&cachedTime)));
                logEvent("Time synced.");
            } else {
                logEvent("Time sync failed");
            }
        } else {
          logEvent("Time sync skip (WiFi disconnected)");
        }
        vTaskDelay(600000 / portTICK_PERIOD_MS); // Пауза 10 минут
    }
}

// Запуск фоновой синхронизации (только один раз)
void startTimeSync() {
    if (timeSyncStarted) return; // Если уже запущено — выходим

    timeSyncStarted = true;
    xTaskCreatePinnedToCore(
        syncTimeTask,  // Функция задачи
        "TimeSyncTask", // Имя
        4096,           // Размер стека
        NULL,           // Параметры
        1,              // Приоритет
        NULL,           // Handle
        0               // Ядро
    );
}
