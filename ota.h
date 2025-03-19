#pragma once

#include <WiFi.h>
//#include <ESPmDNS.h>
//#include <NetworkUdp.h>
#include <ArduinoOTA.h>

void configureOTA();
void checkWiFiAndOTA();
void handleOTA();