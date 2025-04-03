#include <Arduino.h>
#include "config_manager.h"
#include "data.h"
#include "indicator.h"
#include "wifi_manager.h"
#include "log_manager.h"
#include "config.h"
#include "time_sync.h"
#include "ota.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Ticker.h>
#include "filesystem.h"

Ticker wifiScanner;
bool scanning = false;

void scanNetworks();

void onWiFiEvent(WiFiEvent_t event);

void setupWiFi() {
    runtimeData.values["WIFI_SSID"] = String(config["WIFI_SSID"]);
    // runtimeData.values["WIFI_PASS"] = String(config["WIFI_PASS"]);
    runtimeData.values["WIFI_PASS"] = "***"; // hide password from web interface

    runtimeData.values["AP_SSID"] = String(config["AP_SSID"]);
    runtimeData.values["AP_PASS"] = String(config["AP_PASS"]);

    runtimeData.values["host_name"] = String(config["host_name"]);
    runtimeData.values["mDNS"] = String(config["host_name"]) + ".local";
    runtimeData.values["NTP_SERVER1"] = String(config["NTP_SERVER1"]);
    runtimeData.values["NTP_SERVER2"] = String(config["NTP_SERVER2"]);


    logEvent("Wi-Fi: trying to connect to " + String(config["WIFI_SSID"]));
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(onWiFiEvent);  // Регистрируем обработчик событий Wi-Fi
    WiFi.begin(String(config["WIFI_SSID"]), String(config["WIFI_PASS"]));

    setIndicatorState(IndicatorState::CONNECTING);

    wifiScanner.attach(30, scanNetworks);
}

void scanNetworks() {
    if (WiFi.getMode() == WIFI_AP && !scanning) {
        scanning = true;
        logEvent("Wi-Fi: Scanning for SSID=" + String(config["WIFI_SSID"]));
        WiFi.scanNetworks(true);  // Асинхронное сканирование
    }
}

void onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            runtimeData.values["ip"] = WiFi.localIP().toString();
            logEvent("Connected to Wi-Fi: " + WiFi.localIP().toString());
            setIndicatorState(IndicatorState::NONE);
            wifiScanner.detach();  // Останавливаем сканирование

            // === ИНИЦИАЛИЗАЦИЯ mDNS ===
            if (!MDNS.begin(String(config["host_name"]))) {  
                logEvent("mDNS setup failed");
            } else {
                logEvent("mDNS started: " + String(config["host_name"]) + ".local");
                MDNS.addService("http", "tcp", 80); // Регистрируем HTTP-сервис
            }

            checkWiFiAndOTA();

            if (NTP_ENABLED) startTimeSync();
            startDownloadTask();
            break;
        
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            logEvent("Wi-Fi disconnected, switching to AP mode");
            setIndicatorState(IndicatorState::AP_MODE);
            WiFi.mode(WIFI_AP);
            if (WiFi.softAP(String(config["AP_SSID"]), String(config["AP_PASS"]))) {
                logEvent("AP mode active: " + String(config["AP_SSID"]) + ", IP: " + WiFi.softAPIP().toString());
                checkWiFiAndOTA();
                wifiScanner.attach(30, scanNetworks);  // Запускаем фоновое сканирование
            } else {
                setIndicatorState(IndicatorState::ERROR);
                logEvent("AP mode failed");
            }
            break;

        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            logEvent("Client connected to AP");
            break;

        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            logEvent("Client disconnected from AP");
            break;

        case ARDUINO_EVENT_WIFI_SCAN_DONE: {
            scanning = false;
            int networksFound = WiFi.scanComplete();
            if (networksFound > 0) {
                for (int i = 0; i < networksFound; i++) {
                    if (WiFi.SSID(i) == String(config["WIFI_SSID"])) {
                        logEvent("Found target Wi-Fi: " + String(config["WIFI_SSID"]) + ", connecting...");
                        WiFi.mode(WIFI_STA);
                        WiFi.begin(String(config["WIFI_SSID"]), String(config["WIFI_PASS"]));

                        // Ждем подключения, максимум 10 секунд
                        unsigned long startAttemptTime = millis();
                        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
                            delay(500);
                            logEvent("Trying to connect... Status: " + String(WiFi.status()));
                        }

                        if (WiFi.status() == WL_CONNECTED) {
                            logEvent("Successfully connected to " + String(config["WIFI_SSID"]));
                            return;
                        } else {
                            logEvent("Failed to connect");
                        }

                        return;
                    }
                }
            }
            WiFi.scanDelete();  // Очищаем результаты сканирования
            break;
        }

        default:
            break;
    }

}

// void handleWiFi() {
//     static unsigned long lastCheck = 0;
//     if (millis() - lastCheck > 10000) {  // Проверка каждые 10 сек
//         lastCheck = millis();
//         if (WiFi.status() != WL_CONNECTED) {
//             logEvent("Wi-Fi lost, reconnecting...");
//             WiFi.disconnect();
//             WiFi.reconnect();
//         }
//     }
// }
