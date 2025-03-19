#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>

// Запуск веб-сервера
void startWebServer();

// Обновление веб-сервера (вызов в loop)
void handleWebServerUpdates();

#endif // WEB_SERVER_H