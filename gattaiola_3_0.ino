#include <WiFi.h>
#include <time.h>
#include <RTClib.h>
#include <Wire.h>

// WiFi Configuration
const char* ssid = "Vodafone-A76077447";
const char* password = "9w2hl39lwmevzuvx";

// NTP Configuration - UTC only, no timezone offset
const char* ntpServer = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";  // Backup NTP server
const long gmtOffset_sec = 0;              // UTC = 0 offset
const int daylightOffset_sec = 0;          // No daylight saving

// RTC DS3231 object
RTC_DS3231 rtc;

// Global variables
bool rtcFound = false;
bool rtcTimeValid = false;
bool ntpSynced = false;
DateTime rtcTime;
time_t ntpTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 WiFi + NTP + RTC DS3231 Sync ===");
  
  // Initialize I2C and RTC
  Wire.begin();
  initializeRTC();
  
  // Check RTC time validity
  checkRTCTime();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Sync with NTP if WiFi connected
  if (WiFi.status() == WL_CONNECTED) {
    syncWithNTP();
  }
  
  // Display final status
  displayTimeStatus();
}

void loop() {
  // Periodic NTP sync every 12 hours
  static unsigned long lastNtpSync = 0;
  if (WiFi.status() == WL_CONNECTED && millis() - lastNtpSync > 43200000) { // 12 hours
    Serial.println("\n--- Periodic NTP Sync ---");
    syncWithNTP();
    lastNtpSync = millis();
  }
  
  // Display time every 10 seconds
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay > 10000) {
    displayCurrentTimes();
    lastDisplay = millis();
  }
  
  // Check WiFi connection
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 30000) { // Check every 30 seconds
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, attempting reconnection...");
      connectToWiFi();
    }
    lastWiFiCheck = millis();
  }
  
  delay(1000);
}

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

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  const int maxAttempts = 30; // 30 second timeout
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n✗ Failed to connect to WiFi!");
    Serial.println("  Will continue with RTC time only");
  }
}

void syncWithNTP() {
  Serial.println("Syncing with NTP server...");
  Serial.print("NTP Server: ");
  Serial.println(ntpServer);
  
  // Configure NTP with UTC (no timezone offset)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2);
  
  struct tm timeinfo;
  int attempts = 0;
  const int maxAttempts = 10;
  
  // Wait for NTP sync
  while (!getLocalTime(&timeinfo) && attempts < maxAttempts) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (getLocalTime(&timeinfo)) {
    ntpTime = mktime(&timeinfo);
    ntpSynced = true;
    
    Serial.println("\n✓ NTP synchronization successful!");
    Serial.print("NTP Time (UTC): ");
    Serial.println(formatTimeStruct(timeinfo));
    
    // Update RTC with NTP time
    if (rtcFound) {
      updateRTCWithNTP(timeinfo);
    }
  } else {
    Serial.println("\n✗ NTP synchronization failed!");
    Serial.println("  Will use RTC time if available");
    ntpSynced = false;
  }
}

void updateRTCWithNTP(struct tm& timeinfo) {
  Serial.println("Updating RTC with NTP time...");
  
  // Create DateTime object from NTP time
  DateTime ntpDateTime(timeinfo.tm_year + 1900, 
                      timeinfo.tm_mon + 1,
                      timeinfo.tm_mday,
                      timeinfo.tm_hour,
                      timeinfo.tm_min,
                      timeinfo.tm_sec);
  
  // Update RTC
  rtc.adjust(ntpDateTime);
  
  // Verify the update
  delay(100);
  DateTime newRtcTime = rtc.now();
  
  Serial.println("✓ RTC updated with NTP time");
  Serial.print("New RTC Time (UTC): ");
  Serial.println(formatDateTime(newRtcTime));
  
  // Update global variables
  rtcTime = newRtcTime;
  rtcTimeValid = true;
}

void displayTimeStatus() {
  Serial.println("\n=== TIME STATUS SUMMARY ===");
  
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  
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
  
  Serial.print("NTP Synced: ");
  Serial.println(ntpSynced ? "Yes" : "No");
  if (ntpSynced) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      Serial.print("System Time (UTC): ");
      Serial.println(formatTimeStruct(timeinfo));
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
  
  // Display system time (NTP synced)
  if (ntpSynced) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      Serial.print("SYS (UTC): ");
      Serial.print(formatTimeStruct(timeinfo));
      Serial.print(" | Unix: ");
      Serial.println(mktime(&timeinfo));
    }
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

// Optional: Function to get time preference (RTC vs System)
time_t getBestAvailableTime() {
  if (ntpSynced) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      return mktime(&timeinfo);
    }
  }
  
  if (rtcFound && rtcTimeValid) {
    return rtc.now().unixtime();
  }
  
  return 0; // No valid time available
}