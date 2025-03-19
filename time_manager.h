#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>

void initTime();
void updateUptime();
bool updateTimeZone();

extern long gmtOffset_sec;
extern int daylightOffset_sec;

#endif // TIME_MANAGER_H
