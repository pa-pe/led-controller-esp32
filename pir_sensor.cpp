#include <Arduino.h>
#include "config.h"
#include "data.h"
#include "pir_sensor.h"
#include "log_manager.h"
#include "led_controller.h"

volatile int pir1_state = -2;
volatile int pir2_state = -2;
volatile int pir1_state_parent = -2;
volatile int pir2_state_parent = -2;
unsigned long lastPir1Time = 0;
unsigned long lastPir2Time = 0;

// to count flips via interrupts
volatile int pir1_change_cnt = 0;
volatile int pir2_change_cnt = 0;
volatile int pir1_change_cnt_parent = 0;
volatile int pir2_change_cnt_parent = 0;
unsigned long pir1_change_time = 0;
unsigned long pir2_change_time = 0;


String stateToText(int state){
    return (state == -1) ? "Disconnected" : (state ? "Motion" : "No motion");
}

void IRAM_ATTR motionISR1() {
  unsigned int nows = millis() / 1000;
  if (pir1_change_time == nows){
    pir1_change_cnt++;
  } else {
    pir1_change_time = nows;
    pir1_change_cnt_parent = pir1_change_cnt;
    pir1_change_cnt = 1;
    // Serial.println("pir1cnt="+String(pir1_change_cnt_parent));
    if (pir1_change_cnt_parent > 10){
      pir1_state = -1;
    }
  }
}

void IRAM_ATTR motionISR2() {
  unsigned int nows = millis() / 1000;
  if (pir2_change_time == nows){
    pir2_change_cnt++;
  } else {
    pir2_change_time = nows;
    pir2_change_cnt_parent = pir2_change_cnt;
    pir2_change_cnt = 2;
    // Serial.println("pir2cnt="+String(pir2_change_cnt_parent));
    if (pir2_change_cnt_parent > 10){
      pir2_state = -1;
    }
  }
}

// checking the presence of the sensor by switching the internal pull-ups
int detectStablePIRState(int pin) {
    pinMode(pin, INPUT_PULLUP);
    delay(10);
    int highState = digitalRead(pin);

    pinMode(pin, INPUT_PULLDOWN);
    delay(10);
    int lowState = digitalRead(pin);

    pinMode(pin, INPUT);

    if (highState != lowState) {
        return -1;  // lack of sensor
    }
    return lowState ? 0 : 1;
}

void initPIR() {
    runtimeData.values["pir1"] = "Checking...";
    runtimeData.values["pir2"] = "Checking...";

    // pinMode(PIR1_PIN, INPUT);
    // pinMode(PIR2_PIN, INPUT);
    // pir1_state = digitalRead(PIR1_PIN);
    // pir2_state = digitalRead(PIR2_PIN);

    pir1_state = detectStablePIRState(PIR1_PIN);
    pir2_state = detectStablePIRState(PIR2_PIN);

    runtimeData.values["pir1"] = stateToText(pir1_state);
    runtimeData.values["pir2"] = stateToText(pir2_state);

    attachInterrupt(digitalPinToInterrupt(PIR1_PIN), motionISR1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIR2_PIN), motionISR2, CHANGE);
}

void handlePIR() {
    unsigned long now = millis();

    if (pir1_state > -1){
      pir1_state = digitalRead(PIR1_PIN);

      if (pir1_state != pir1_state_parent){ // change
          lastPir1Time = now;
          pir1_state_parent = pir1_state;
          runtimeData.values["pir1"] = stateToText(pir1_state);
          logEvent("PIR1 state changed: " + runtimeData.values["pir1"]);
      }
    }

    if (pir2_state > -1){
      pir2_state = digitalRead(PIR2_PIN);

      if (pir2_state != pir2_state_parent){ // change
          lastPir2Time = now;
          pir2_state_parent = pir2_state;
          runtimeData.values["pir2"] = stateToText(pir2_state);
          logEvent("PIR2 state changed: " + runtimeData.values["pir2"]);
      }
    }
}
