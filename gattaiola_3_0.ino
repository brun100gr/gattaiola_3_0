#include "rtc.h"
#include "utilities.h"


void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 WiFi + NTP + RTC DS3231 Sync ===");
  
  // Initialize I2C and RTC
  Wire.begin();
  initializeRTC();
  
  // Check RTC time validity
  checkRTCTime();
  
  // Display final status
  displayTimeStatus();
}

void loop() {
  // Display time every 10 seconds
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay > 10000) {
    displayCurrentTimes();
    lastDisplay = millis();
  }
  
  delay(1000);
}

void displayTimeStatus() {
  Serial.println("\n=== TIME STATUS SUMMARY ===");
  
  Serial.print("RTC Found: ");
  Serial.println(rtcFound ? "Yes" : "No");
  
  if (rtcFound) {
    Serial.print("RTC Time Valid: ");
    Serial.println(rtcTimeValid ? "Yes" : "No");
    if (rtcTimeValid) {
      Serial.print("RTC Time (UTC): ");
      Serial.println(formatDateTime(rtc.now()));
    }
  }
  
  Serial.println("========================\n");
}

void displayCurrentTimes() {
  Serial.println("--- Current Times ---");
  
  // Display RTC time
  if (rtcFound && rtcTimeValid) {
    DateTime now = rtc.now();
    Serial.print("RTC (UTC): ");
    Serial.print(formatDateTime(now));
    Serial.print(" | Unix: ");
    Serial.println(now.unixtime());
  }
  
  // Display uptime
  Serial.print("Uptime: ");
  unsigned long uptimeSeconds = millis() / 1000;
  unsigned long hours = uptimeSeconds / 3600;
  unsigned long minutes = (uptimeSeconds % 3600) / 60;
  unsigned long seconds = uptimeSeconds % 60;
  Serial.printf("%02lu:%02lu:%02lu\n", hours, minutes, seconds);
  Serial.println();
}

String formatTimeStruct(struct tm& timeinfo) {
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

// Optional: Function to manually set RTC time (useful for testing)
void setRTCTime(int year, int month, int day, int hour, int minute, int second) {
  if (!rtcFound) {
    Serial.println("RTC not available");
    return;
  }
  
  DateTime newTime(year, month, day, hour, minute, second);
  rtc.adjust(newTime);
  
  Serial.println("RTC time manually set to:");
  Serial.println(formatDateTime(newTime));
  
  rtcTimeValid = true;
}
