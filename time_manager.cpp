#include "time_manager.h"
#include "config_manager.h"
#include "data.h"
#include "log_manager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "time_sync.h"

long gmtOffset_sec = 0;
int daylightOffset_sec = 0;

bool updateTimeZone() {
    bool result = false;

    logEvent("Trying get Time zone");

    HTTPClient http;
    http.begin("http://worldtimeapi.org/api/ip");
    
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
        String payload = http.getString();
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            // Получаем строку смещения, например "-05:00"
            String offsetStr = doc["utc_offset"].as<String>();
            runtimeData.values["timezone"] = offsetStr;

            // Парсим часы и минуты
            int hours = offsetStr.substring(0, 3).toInt(); // Первые 3 символа: "-05"
            int minutes = offsetStr.substring(4, 6).toInt(); // Минуты (обычно 00, но могут быть 30 или 45)

            // Преобразуем в секунды
            // gmtOffset_sec = (hours * 3600) + (abs(minutes) * 60);
            gmtOffset_sec = (hours * 3600) + (minutes * 60);
           
            daylightOffset_sec = doc["dst"].as<bool>() ? 3600 : 0;

            result = true;

            logEvent("Time zone updated: GMT " + String(gmtOffset_sec / 3600));
        } else {
            logEvent("JSON parsing failed for time zone");
        }
    } else {
        logEvent("Failed to get timezone, HTTP error: " + String(httpResponseCode));
    }
    
    http.end();

    return result;
}

// void initTime() {
//     if (WiFi.status() == WL_CONNECTED) {
//         if (!updateTimeZone()) updateTimeZone();

//         configTime(gmtOffset_sec, daylightOffset_sec, String(config["NTP_SERVER"]).c_str());
//         struct tm timeInfo;
//         if (getLocalTime(&timeInfo)) {
//             logEvent("Time synchronized via NTP " + String(config["NTP_SERVER"]));
//         } else {
//             logEvent("Failed to sync time via NTP " + String(config["NTP_SERVER"]));
//         }
//     } else {
//         logEvent("No WiFi, skipping time sync");
//     }
// }

void updateUptime() {
    unsigned long uptime = millis() / 1000;
    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;

    runtimeData.values["uptime"] = String(days) + "d " + String(hours) + "h " + 
                                   String(minutes) + "m " + String(seconds) + "s";

    if (!lastMillis) return;

    // Создаём копию
    struct tm cachedTime_ = cachedTime;

    // Смещение относительно последнего обновления
    unsigned long deltaMillis = millis() - lastMillis;
    cachedTime_.tm_sec += deltaMillis / 1000;

    // Коррекция минут и часов
    if (cachedTime_.tm_sec >= 60) {
        cachedTime_.tm_min += cachedTime_.tm_sec / 60;
        cachedTime_.tm_sec %= 60;
    }
    if (cachedTime_.tm_min >= 60) {
        cachedTime_.tm_hour += cachedTime_.tm_min / 60;
        cachedTime_.tm_min %= 60;
    }
    if (cachedTime_.tm_hour >= 24) {
        cachedTime_.tm_mday += cachedTime_.tm_hour / 24;
        cachedTime_.tm_hour %= 24;
    }

    // Применяем смещение часового пояса
    // time_t localTimestamp = mktime(&cachedTime_) + gmtOffset_sec + daylightOffset_sec;
    // struct tm *localTime = localtime(&localTimestamp);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &cachedTime_);
    // strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    runtimeData.values["datetime"] = buffer;

    if (runtimeData.values["timezone"] == ""){
        runtimeData.values["timezone"] = "UTC";
    }
}
