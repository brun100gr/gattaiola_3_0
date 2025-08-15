#ifndef RTC_H
#define RTC_H

#include <time.h>
#include <RTClib.h>
#include <Wire.h>
#include "utilities.h"

// RTC DS3231 object
RTC_DS3231 rtc;

// Global variables
bool rtcFound = false;
bool rtcTimeValid = false;
DateTime rtcTime;

void initializeRTC() {
  Serial.println("Initializing RTC DS3231...");
  
  if (!rtc.begin()) {
    Serial.println("✗ Could not find RTC DS3231!");
    Serial.println("  Check wiring: SDA->GPIO21, SCL->GPIO22, VCC->3.3V, GND->GND");
    rtcFound = false;
    return;
  }
  
  rtcFound = true;
  Serial.println("✓ RTC DS3231 found");
  
  // Check if RTC lost power
  if (rtc.lostPower()) {
    Serial.println("⚠ RTC lost power - time may be invalid");
    rtcTimeValid = false;
  } else {
    Serial.println("✓ RTC power was maintained");
  }
  
  // Display RTC info
  float temperature = rtc.getTemperature();
  Serial.print("RTC Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
}

void checkRTCTime() {
  if (!rtcFound) {
    Serial.println("RTC not available - skipping time check");
    return;
  }
  
  Serial.println("Checking RTC time validity...");
  
  rtcTime = rtc.now();
  
  // Check if time is reasonable (after year 2020)
  if (rtcTime.year() >= 2020) {
    rtcTimeValid = true;
    Serial.println("✓ RTC time appears valid");
    Serial.print("RTC Time (UTC): ");
    Serial.println(formatDateTime(rtcTime));
  } else {
    rtcTimeValid = false;
    Serial.println("✗ RTC time appears invalid (year < 2020)");
    Serial.print("RTC Time: ");
    Serial.println(formatDateTime(rtcTime));
  }
}

#endif // RTC_H