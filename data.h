#pragma once

#include <Arduino.h>
#include <map>

class RuntimeData {
public:
    std::map<String, String> values;  // Динамическое хранилище оперативных данных

    RuntimeData();  // Конструктор для заполнения значений по умолчанию
};

extern RuntimeData runtimeData;
