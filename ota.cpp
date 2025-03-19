#include "ota.h"
#include "log_manager.h"

void configureOTA() {

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      logEvent("Start updating " + type);
    })
    .onEnd([]() {
      logEvent("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        logEvent("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        logEvent("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        logEvent("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        logEvent("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        logEvent("End Failed");
      }
    });

//  ArduinoOTA.begin();
}

void checkWiFiAndOTA() {
    if (WiFi.status() == WL_CONNECTED) {
        static bool otaStarted = false;
        if (!otaStarted) {
            ArduinoOTA.begin();
            otaStarted = true;
            logEvent("OTA initialized");
        }
    }
}


void handleOTA() {
    ArduinoOTA.handle();
}