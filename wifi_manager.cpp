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
#include <Ticker.h>
// #include <NetworkClient.h>
// #include <WiFiAP.h>

Ticker wifiScanner;
bool scanning = false;

void scanNetworks();

void onWiFiEvent(WiFiEvent_t event);

void setupWiFi() {
    runtimeData.values["WIFI_SSID"] = String(config["WIFI_SSID"]);
    // runtimeData.values["WIFI_PASS"] = String(config["WIFI_PASS"]);
    runtimeData.values["WIFI_PASS"] = "***";

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
        logEvent("Scanning for known Wi-Fi networks...");
        WiFi.scanNetworks(true);  // Асинхронное сканирование
    }
}

void onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            logEvent("Connected to Wi-Fi: " + WiFi.localIP().toString());
            setIndicatorState(IndicatorState::NONE);
            wifiScanner.detach();  // Останавливаем сканирование в STA режиме
            startTimeSync(); // Запускаем синхронизацию времени при подключении к Wi-Fi
            checkWiFiAndOTA();
            break;
        
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            logEvent("Wi-Fi disconnected, switching to AP mode");
            setIndicatorState(IndicatorState::AP_MODE);
            WiFi.mode(WIFI_AP);
            if (WiFi.softAP(AP_SSID, AP_PASS)) {
                logEvent("AP mode active: " + String(AP_SSID) + ", IP: " + WiFi.softAPIP().toString());
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

// void setupWiFi() {
//     logEvent("Wi-Fi: try connect to " + String(WIFI_SSID));
//     WiFi.mode(WIFI_STA);  // Сначала пробуем подключение как клиент
//     WiFi.begin(WIFI_SSID, WIFI_PASS);
    
//     setIndicatorState(IndicatorState::CONNECTING);
    
//     if (WiFi.waitForConnectResult() != WL_CONNECTED) {
//         logEvent("Wi-Fi: connect to " + String(WIFI_SSID) + " filed, enabling AP mode");

//         // WiFi.mode(WIFI_AP_STA);  // Включаем STA+AP одновременно
//         WiFi.mode(WIFI_AP);
//        if (WiFi.softAP(AP_SSID, AP_PASS)){
//           setIndicatorState(IndicatorState::AP_MODE);
//           logEvent("AP mode active: " + String(AP_SSID)+ ", IP: " + WiFi.softAPIP().toString());
//         } else {
//           setIndicatorState(IndicatorState::ERROR);
//           logEvent("AP mode fail");
//         }

//     } else {
//         setIndicatorState(IndicatorState::NONE);
//         logEvent("Connected to Wi-Fi: " + WiFi.localIP().toString());
//     }

//     // Регистрируем обработчик событий Wi-Fi
//     WiFi.onEvent(onWiFiEvent);
// }

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

// void onWiFiEvent(WiFiEvent_t event) {
//     switch (event) {
//         case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
//             logEvent("Wi-Fi disconnected, trying to reconnect...");
//             setIndicatorState(IndicatorState::CONNECTING);
//             WiFi.reconnect();
//             break;

//         case ARDUINO_EVENT_WIFI_STA_CONNECTED:
//             logEvent("Wi-Fi connected to AP");
//             break;

//         case ARDUINO_EVENT_WIFI_STA_GOT_IP:
//             logEvent("Wi-Fi connected, IP: " + WiFi.localIP().toString());
//             setIndicatorState(IndicatorState::NONE);
//             break;

//         case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
//             logEvent("Client connected to AP");
//             break;

//         case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
//             logEvent("Client disconnected from AP");
//             break;
//     }
// }



// void handleWiFi() {
//     static unsigned long lastBlink = 0;
//     if (WiFi.getMode() == WIFI_MODE_AP && millis() - lastBlink > 3000) {
//         digitalWrite(STATUS_LED, HIGH);
//         delay(100);
//         digitalWrite(STATUS_LED, LOW);
//         delay(100);
//         lastBlink = millis();
//     }
// }
