#ifndef CONFIG_H
#define CONFIG_H

// hardware
#define PIR1_PIN 3
#define PIR2_PIN 4

#define LED_PIN 10 // LED strip
#define STATUS_LED 8  // LED для индикации
#define STATUS_LED_ON_STATE 0 // BUILT_IN_LED может включаться как от LOW так и от HIGH на разном hardware
#define LED_ON_STATE 1

#define AP_SSID "led-controller-esp"
#define AP_PASS "12345678"

#define OTA_ENABLED 1 // Over-the-air update

// log
#define LOG_SIZE 15  // Храним 15 последних событий

#endif
