#include "filesystem.h"
#include "log_manager.h"
#include "SPIFFS.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <freertos/queue.h>
#include "esp_task_wdt.h"

// Структура для сопоставления файлов
struct FileMapping {
    const char *path;
    const char *downloadURL;
};

// Список файлов для скачивания
FileMapping files[] = {
    {"/bootstrap-grid.min.css.gz", "https://raw.githubusercontent.com/pa-pe/led-controller-esp32/refs/heads/main/data/bootstrap-grid.min.css.gz"},
    // {"/bootstrap.min.css.gz", "https://raw.githubusercontent.com/twbs/bootstrap/main/dist/css/bootstrap.min.css"},
    // {"/style.css.gz", "https://example.com/style.css"}
};

QueueHandle_t downloadQueue = NULL;
TaskHandle_t downloadTaskHandle = NULL;

// Функция проверки существования файла
bool fileExists(const char* path) {
    return SPIFFS.exists(path);
}

// Фоновая задача для загрузки файлов
void downloadTask(void *param) {
    FileMapping file;

    while (xQueueReceive(downloadQueue, &file, portMAX_DELAY) == pdPASS) {
        // Проверяем подключение перед загрузкой
        if (WiFi.status() != WL_CONNECTED) {
            logEvent("Skipping download, no internet.");
            continue;
        }

        logEvent("Downloading: " + String(file.downloadURL));
        HTTPClient http;
        http.begin(file.downloadURL);
        int httpResponseCode = http.GET();

        if (httpResponseCode == 200) {
            File f = SPIFFS.open(file.path, "w");
            if (f) {
                f.write(http.getStream());
                f.close();
                logEvent("Downloaded: " + String(file.path));
            } else {
                logEvent("Failed to save: " + String(file.path));
            }
        } else {
            logEvent("Download failed: " + String(file.downloadURL) + " Code: " + String(httpResponseCode));
        }

        http.end();
    }

    // Очередь пустая – завершаем задачу
    vQueueDelete(downloadQueue);
    downloadQueue = NULL;
    downloadTaskHandle = NULL;
    vTaskDelete(NULL);
}

// Функция запуска задачи скачивания
void startDownloadTask() {
    if (WiFi.status() != WL_CONNECTED) {
        logEvent("Cannot start download task, no internet connection.");
        return;
    }

    if (!downloadQueue || uxQueueMessagesWaiting(downloadQueue) == 0) {
        logEvent("No files to download.");
        return;
    }

    if (!downloadTaskHandle) {
        xTaskCreate(downloadTask, "DownloadTask", 8192, NULL, 1, &downloadTaskHandle);
        logEvent("Download task started.");
    }
}

// Инициализация файловой системы и проверка файлов
void initFileSystem() {
    if (!SPIFFS.begin(true)) {
        logEvent("SPIFFS Mount Failed");
        return;
    }

    // Создаем очередь, если ее еще нет
    if (!downloadQueue) {
        downloadQueue = xQueueCreate(5, sizeof(FileMapping));
    }

    // Проверяем, какие файлы нужно скачать
    for (auto &file : files) {
        if (!SPIFFS.exists(file.path) && file.downloadURL) {
            xQueueSend(downloadQueue, &file, portMAX_DELAY);
        }
    }

    // Запускаем загрузку
    // startDownloadTask();
    // запуск загрузки перенесен в wifi_manager.cpp на onWiFiEvent ARDUINO_EVENT_WIFI_STA_GOT_IP
}

bool formatSPIFFS() {
  logEvent("Formatting SPIFFS...");
        
  // Отписываем текущую задачу от WDT
  esp_task_wdt_delete(NULL);

  // Отключаем Watchdog Timer перед форматированием
  esp_task_wdt_deinit();

  bool success = SPIFFS.format();

  // Включаем Watchdog обратно
  esp_task_wdt_init(nullptr);

  // Повторно подписываем задачу на WDT
  esp_task_wdt_add(NULL);

  if (success) {
      logEvent("SPIFFS formatted successfully");
      // ESP.restart();
  } else {
      logEvent("SPIFFS format failed");
  }

  return success;
}