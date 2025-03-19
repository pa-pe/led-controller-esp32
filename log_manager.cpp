#include <Arduino.h>

#include "config.h"
#include "data.h"
#include "log_manager.h"
#include "time_manager.h"

String logBuffer[LOG_SIZE];
int logIndex = 0;

void logEvent(const String& event) {
    updateUptime();
    if (runtimeData.values["datetime"] != ""){
      logBuffer[logIndex] = runtimeData.values["datetime"] + " " + runtimeData.values["timezone"] + ": " + event;
    } else {
      logBuffer[logIndex] = runtimeData.values["uptime"] + ": " + event;
    }
    Serial.println(logBuffer[logIndex]);
    logIndex = (logIndex + 1) % LOG_SIZE;
    // Serial.println(event);
    runtimeData.values["log"] = getLogHistory("\\n");
}

String getLogHistory(const String& delimiter) {
    String history;
    int i = logIndex;
    for (int count = 0; count < LOG_SIZE; count++) {
        if (logBuffer[i] != "") {
            history += logBuffer[i] + delimiter;
        }
        i = (i + 1) % LOG_SIZE;
    }
    return history;
}
