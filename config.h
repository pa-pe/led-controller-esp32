#pragma once

// hardware
#define PIR1_PIN 3
#define PIR2_PIN 4

#define LED_PIN 10 // LED strip
#define STATUS_LED 8  // LED для индикации
#define STATUS_LED_ON_STATE 0 // BUILT_IN_LED может включаться как от LOW так и от HIGH на разном hardware
#define LED_ON_STATE 1

#define OTA_ENABLED 1 // Over-the-air update
#define NTP_ENABLED 0 // Network clock synchronization

// log
#define LOG_SIZE 15  // Храним 15 последних событий


// The best way is to not touch the default settings and instead update the web configuration
#define DEFAULT_NAME "led-controller" // used for host_name && AP_SSID
#define DEFAULT_AP_PASS "12345678"
