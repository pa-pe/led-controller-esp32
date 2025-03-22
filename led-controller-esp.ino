#include "config.h"
#include "config_manager.h"
#include "indicator.h"
#include "filesystem.h"
#include "wifi_manager.h"
#include "ota.h"
#include "time_manager.h"
#include "pir_sensor.h"
#include "led_controller.h"
#include "web_server.h"
#include "log_manager.h"

void setup() {
    Serial.begin(115200);

    loadConfig();
    
    initIndicator();
    initFileSystem(); // important before WiFi and WebServer
    setupWiFi();
    startWebServer();
    initLED();
    initPIR();
    if (OTA_ENABLED) configureOTA();
}

// void loop() {
//     updateUptime();
//     updateIndicator();
//     handlePIR();
//     handleLED();
//     // handleWiFi();
//     handleWebServerUpdates();
//     delay(50);
// }

int loop_delay = 50;
void loop() {
    unsigned long start = millis();
    measureExecutionTime(updateUptime, "updateUptime");
    measureExecutionTime(updateIndicator, "updateIndicator");
    measureExecutionTime(handlePIR, "handlePIR");
    measureExecutionTime(handleLED, "handleLED");
    measureExecutionTime(handleWebServerUpdates, "handleWebServerUpdates");
    if (OTA_ENABLED) measureExecutionTime(handleOTA, "handleOTA");

    int delta = start - millis();
    if (delta < loop_delay){
      delay(loop_delay - delta);
    }
    // delay(loop_delay);
}

void measureExecutionTime(void (*func)(), const char *funcName) {
    unsigned long start = millis();
    func();
    unsigned long duration = millis() - start;
    if (duration > 5) {  // Выводим только если выполнение заняло больше 5 мс
        Serial.printf("[WARNING] %s took %lu ms\n", funcName, duration);
    }
}
