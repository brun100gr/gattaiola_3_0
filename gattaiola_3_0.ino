#include "rtc.h"
#include "utilities.h"
#include "sleep.h"
#include <cstdint>

// Configuration constants
#define WAKE_PIN GPIO_NUM_0        // GPIO0 (BOOT button) for external wake
#define WAKE_PIN_LEVEL 0           // Wake when pin goes LOW (button pressed)

const uint32_t mS_TO_S_FACTOR = 1000;  // Conversion factor for milliseconds to seconds
const uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for microseconds to seconds

const uint32_t ONE_SECOND = 1 * mS_TO_S_FACTOR;
const uint32_t UP_TIME = 30 * mS_TO_S_FACTOR;
const uint64_t SLEEP_TIME_1_MIN = 60 * uS_TO_S_FACTOR;

const uint32_t LED_BUILTIN = 2;  // Most ESP32 boards have builtin LED on GPIO2

// Timing variables
unsigned long previousMillis = 0;
const unsigned long BLINK_INTERVAL = 250;  // 250ms = 2Hz (ON 250ms, OFF 250ms)

bool rtcError = true;

// LED state
bool ledState = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 WiFi + NTP + RTC DS3231 Sync ===");

  // Initialize builtin LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Start with LED OFF

  // Print wake up reason
  printWakeupReason();

  // Initialize I2C and RTC
  Wire.begin();
  initializeRTC();
  
  // Check RTC time validity
  rtcError != checkRTCTime();

  // Blink built-in LED in case of error
  if (rtcError) {
    handleLEDBlink();
  }
  
  // Display final status
  displayTimeStatus();
}

void loop() {
  // Handle LED blinking (non-blocking)
  handleLEDBlink();

  // Display time every 10 seconds
  static unsigned long lastDisplay = 0;
  static unsigned long upTime = 0;
  if (millis() - lastDisplay > ONE_SECOND) {
    displayCurrentTimes();
    lastDisplay = millis();
  }
  
  if (millis() - upTime > UP_TIME) {
    // Enter deep sleep with multiple wake sources
    enterDeepSleep(SLEEP_TIME_1_MIN, true, true);
  }
}

void handleLEDBlink() {
  unsigned long currentMillis = millis();
  
  // Check if it's time to toggle the LED
  if (currentMillis - previousMillis >= BLINK_INTERVAL) {
    // Save the last time LED was toggled
    previousMillis = currentMillis;
    
    // Toggle LED state
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  }
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

