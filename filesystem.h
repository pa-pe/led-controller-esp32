#pragma once

#include <Arduino.h>

void initFileSystem();
bool fileExists(const char* path);
void startDownloadTask();