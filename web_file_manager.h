#ifndef WEB_FILE_MANAGER_H
#define WEB_FILE_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

void setupFileManager(AsyncWebServer &server, String title);
// void logEvent(const String &message);

#endif // WEB_FILE_MANAGER_H
