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


String stateToText(int state){
    return (state == -1) ? "Disconnected" : (state ? "Motion" : "No motion");
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

    pir1_state = detectStablePIRState(PIR1_PIN);
    pir2_state = detectStablePIRState(PIR2_PIN);

    runtimeData.values["pir1"] = stateToText(pir1_state);
    runtimeData.values["pir2"] = stateToText(pir2_state);
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
