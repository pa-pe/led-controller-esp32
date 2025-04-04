#include "config.h"
#include "config_manager.h"
#include <ArduinoJson.h>
#include <Preferences.h>

StaticJsonDocument<256> config;
Preferences preferences; 

void loadConfig() {
    preferences.begin("config", true); // read only
    // config["wifi_ssid"] = preferences.getString("wifi_ssid", "your-ssid").c_str();
    // config["wifi_password"] = preferences.getString("wifi_password", "your-password").c_str();
    config["host_name"] = preferences.getString("host_name", DEFAULT_NAME).c_str();
    config["AP_SSID"] = preferences.getString("AP_SSID", DEFAULT_NAME).c_str();
    config["AP_PASS"] = preferences.getString("AP_SSID", DEFAULT_AP_PASS).c_str();
    config["led_mode"] = preferences.getString("led_mode", "pir").c_str(); // default: pir
    config["led_start_from"] = preferences.getInt("led_start_from", 0);
    config["led_max"] = preferences.getInt("led_max", 100);
    config["led_delay"] = preferences.getInt("led_delay", 5); // default 5 sec
    config["WIFI_SSID"] = preferences.getString("WIFI_SSID", "REPLACE_WIFI_SSID").c_str(); // default: WIFI_SSID
    config["WIFI_PASS"] = preferences.getString("WIFI_PASS", "REPLACE_WIFI_PASS").c_str(); // default: WIFI_PASS
    config["NTP_SERVER1"] = preferences.getString("NTP_SERVER1", "us.pool.ntp.org").c_str();
    config["NTP_SERVER2"] = preferences.getString("NTP_SERVER2", "pool.ntp.org").c_str();
    preferences.end();
}

void saveConfig() {
    preferences.begin("config", false); // Open for write
    // preferences.putString("wifi_ssid", String(config["wifi_ssid"]).c_str());
    // preferences.putString("wifi_password", String(config["wifi_password"]).c_str());
    preferences.putString("host_name", String(config["host_name"]).c_str());
    preferences.putString("AP_SSID", String(config["AP_SSID"]).c_str());
    preferences.putString("AP_PASS", String(config["AP_PASS"]).c_str());
    preferences.putString("led_mode", String(config["led_mode"]).c_str());
    preferences.putInt("led_start_from", config["led_start_from"].as<int>());
    preferences.putInt("led_max", config["led_max"].as<int>());
    preferences.putInt("led_delay", config["led_delay"].as<int>());
    preferences.putString("WIFI_SSID", String(config["WIFI_SSID"]).c_str());
    preferences.putString("WIFI_PASS", String(config["WIFI_PASS"]).c_str());
    preferences.putString("NTP_SERVER1", String(config["NTP_SERVER1"]).c_str());
    preferences.putString("NTP_SERVER2", String(config["NTP_SERVER2"]).c_str());
    preferences.end();
}

const char* getConfigValue(const char* key) {
    return config[key];
}

int getConfigValueInt(const char* key) {
    return config[key] | 0;
}

void setConfigValue(const char* key, const char* value) {
    config[key] = value;
    saveConfig();
}

void setConfigValueInt(const char* key, int value) {
    config[key] = value;
    saveConfig();
}
