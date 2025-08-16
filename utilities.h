#ifndef UTILITIES_H
#define UTILITIES_H

// Utility functions
String formatDateTime(DateTime dt) {
  char buffer[25];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           dt.year(), dt.month(), dt.day(),
           dt.hour(), dt.minute(), dt.second());
  return String(buffer);
}

String formatTimeStruct(struct tm& timeinfo) {
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

#endif // UTILITIES_H
