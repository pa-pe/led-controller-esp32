#include "led_controller.h"
#include "config.h"
#include "config_manager.h"
#include "data.h"
#include "log_manager.h"
#include <Arduino.h>

int currentBrightness = 0;
int targetBrightness = 0;
unsigned int onTo = 0;

void initLED() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, !LED_ON_STATE);
    runtimeData.values["led_start_from"] = String(config["led_start_from"]);
    runtimeData.values["led_max"] = String(config["led_max"]);
    runtimeData.values["led_delay"] = String(config["led_delay"]);
    runtimeData.values["led_mode"] = String(config["led_mode"]);
}

void setLEDBrightness(int brightness) {
    targetBrightness = brightness;
}

void handleLED() {
    unsigned int nows = millis() / 1000;

    if (runtimeData.values["led_mode"] == "pir"){
      if (runtimeData.values["pir1"] == "Motion" or runtimeData.values["pir2"] == "Motion"){
        onTo = nows + config["led_delay"].as<int>();
      }

      if (onTo >= nows){
        if (targetBrightness != config["led_max"]){
          targetBrightness = config["led_max"];
          logEvent("LED set by PIR: " + String(targetBrightness));
        }
      } else {
        if (targetBrightness != 0){
          targetBrightness = 0;
          logEvent("LED set by PIR: " + String(targetBrightness));
        }
      }
    } else if (runtimeData.values["led_mode"] == "manual"){
      // do nothing
    }

    if (currentBrightness != targetBrightness) {
        currentBrightness += (targetBrightness > currentBrightness) ? 5 : -5;
        runtimeData.values["currentBrightness"] = String(currentBrightness);
        int ledStartFrom = map(config["led_start_from"], 0, 100, 0, 255);
        int setAnalogValue = map(currentBrightness, 0, 100, ledStartFrom, 255); // percent to byte
        if (LED_ON_STATE == 0){
            setAnalogValue = map(currentBrightness, 0, 255, 255, 0); // invert
        }

        pinMode(LED_PIN, OUTPUT);
        pinMode(STATUS_LED, OUTPUT);

        if (currentBrightness == 100){
          // Serial.println(String(currentBrightness) + "% digitalWrite=" + LED_ON_STATE);
          digitalWrite(LED_PIN, LED_ON_STATE);
          digitalWrite(STATUS_LED, STATUS_LED_ON_STATE);
        } else if (currentBrightness == 0) {
          // Serial.println(String(currentBrightness) + "% digitalWrite=" + !LED_ON_STATE);
          digitalWrite(LED_PIN, !LED_ON_STATE);
          digitalWrite(STATUS_LED, !STATUS_LED_ON_STATE);
        } else {
          // Serial.println(String(currentBrightness) + "% analog=" + String(setAnalogValue));
          analogWrite(LED_PIN, setAnalogValue);
          if (LED_ON_STATE == STATUS_LED_ON_STATE){
            analogWrite(STATUS_LED, setAnalogValue);
          } else {
            analogWrite(STATUS_LED, map(setAnalogValue, 0, 255, 255, 0));
          }
        }

    }
}
