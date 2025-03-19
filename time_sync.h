#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

// Глобальная переменная для хранения системного времени
extern struct tm cachedTime;

// Время последнего обновления миллисекунд
extern unsigned long lastMillis;

// Флаг, запущена ли синхронизация
extern bool timeSyncStarted;

// Запуск фоновой синхронизации
void startTimeSync();

#endif // TIME_SYNC_H
