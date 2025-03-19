#pragma once
// #ifndef INDICATOR_H
// #define INDICATOR_H


enum class IndicatorState {
    NONE,
    AP_MODE,
    CONNECTING,
    ERROR
};

void initIndicator();
void setIndicatorState(IndicatorState state);
IndicatorState getIndicatorState();
void updateIndicator();

// #endif
