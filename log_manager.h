#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

// void logEvent(String event);
void logEvent(const String& event);
String getLogHistory(const String& delimiter = "\n");

#endif
