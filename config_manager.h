#pragma once

#include <ArduinoJson.h>
#include <Preferences.h>

extern StaticJsonDocument<256> config;
extern Preferences preferences;

void loadConfig();
void saveConfig();
const char* getConfigValue(const char* key);
void setConfigValue(const char* key, const char* value);
void setConfigValueInt(const char* key, int value);