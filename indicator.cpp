#include <Arduino.h>
#include "config.h"
#include "indicator.h"

constexpr uint16_t INDICATOR_REPEAT_MS = 3000;  // Интервал повторения

struct BlinkPattern {
    uint8_t blinks;
    uint16_t onTime;
    uint16_t offTime;
    bool repeat;
};

const BlinkPattern patterns[] = {
    {0, 0, 0, false},    // NONE
    {3, 150, 150, true}, // AP_MODE
    {2, 150, 150, true}, // CONNECTING
    {5, 150, 150, false} // ERROR
};

static IndicatorState currentState = IndicatorState::NONE;
static uint8_t blinkCount = 0;
static unsigned long lastBlinkTime = 0;
static bool ledState = false;

void setIndicatorState(IndicatorState state) {
    currentState = state;
    blinkCount = 0;
    lastBlinkTime = millis();
}

IndicatorState getIndicatorState() {
    return currentState;
}

// void updateIndicator() {
//     if (currentState == IndicatorState::NONE) return;

//     const BlinkPattern& pattern = patterns[static_cast<uint8_t>(currentState)];
//     unsigned long now = millis();

//     if (blinkCount < pattern.blinks * 2) {
//         if (now - lastBlinkTime >= (ledState ? pattern.onTime : pattern.offTime)) {
//             ledState = !ledState;
//             digitalWrite(STATUS_LED, ledState ? HIGH : LOW);
//             lastBlinkTime = now;
//             blinkCount++;
//         }
//     } else if (pattern.repeat) {
//         if (now - lastBlinkTime >= INDICATOR_REPEAT_MS) {
//             blinkCount = 0;
//         }
//     } else {
//         currentState = IndicatorState::NONE;  // Отключаем после одного цикла
//     }
// }

// void updateIndicator() {
//     if (currentState == IndicatorState::NONE) {
//         // digitalWrite(STATUS_LED, HIGH);  // Гасим LED
//         return;
//     }

//     const BlinkPattern& pattern = patterns[static_cast<uint8_t>(currentState)];
//     unsigned long now = millis();

//     if (blinkCount < pattern.blinks * 2) {
//         if (now - lastBlinkTime >= (ledState ? pattern.onTime : pattern.offTime)) {
//             ledState = !ledState;
//             digitalWrite(STATUS_LED, ledState ? HIGH : LOW);
//             lastBlinkTime = now;
//             blinkCount++;
//         }
//     } else if (pattern.repeat && now - lastBlinkTime >= INDICATOR_REPEAT_MS) {
//         blinkCount = 0;  // Перезапуск мигания
//     } else if (!pattern.repeat) {
//         digitalWrite(STATUS_LED, HIGH);  // Гасим LED после завершения
//         currentState = IndicatorState::NONE;
//     }
// }

void updateIndicator() {
    static bool indicatorOff = false; // Флаг для контроля состояния LED

    if (currentState == IndicatorState::NONE) {
        if (!indicatorOff) { 
            digitalWrite(STATUS_LED, !STATUS_LED_ON_STATE); // Гасим LED только один раз
            indicatorOff = true; // Запоминаем, что выключили
        }
        return;
    }

    indicatorOff = false; // Если не NONE, сбрасываем флаг для следующего выключения

    const BlinkPattern& pattern = patterns[static_cast<uint8_t>(currentState)];
    unsigned long now = millis();

    if (blinkCount == 0) {
        digitalWrite(STATUS_LED, !STATUS_LED_ON_STATE);
    }

    if (blinkCount < pattern.blinks * 2) {
        if (now - lastBlinkTime >= (ledState ? pattern.onTime : pattern.offTime)) {
            ledState = !ledState;
            digitalWrite(STATUS_LED, ledState ? STATUS_LED_ON_STATE : !STATUS_LED_ON_STATE);
            lastBlinkTime = now;
            blinkCount++;
        }
    } else if (pattern.repeat) {
        if (now - lastBlinkTime >= INDICATOR_REPEAT_MS) {
            blinkCount = 0; 
            digitalWrite(STATUS_LED, !STATUS_LED_ON_STATE);
        }
    } else {
        digitalWrite(STATUS_LED, !STATUS_LED_ON_STATE);
        currentState = IndicatorState::NONE;
    }
}


void initIndicator(){
  pinMode(STATUS_LED, OUTPUT);
  setIndicatorState(IndicatorState::NONE);
}
