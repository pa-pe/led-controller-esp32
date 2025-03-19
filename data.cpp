#include "data.h"

RuntimeData::RuntimeData() {
    values["pir1"] = "-1"; 
    values["pir2"] = "-1"; 
    values["currentBrightness"] = "0";
    values["log"] = "";             
    values["datetime"] = "";
    values["timezone"] = "";
}

RuntimeData runtimeData;
