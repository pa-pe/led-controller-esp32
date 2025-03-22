#include "spiffs.h"
#include "log_manager.h"
#include "SPIFFS.h"

void setupSPIFFS() {
    if (!SPIFFS.begin(true)) { // true = автоформатирование при ошибке
        logEvent("SPIFFS Mount Failed");
        return;
    }
    logEvent("SPIFFS Mounted Successfully");
}