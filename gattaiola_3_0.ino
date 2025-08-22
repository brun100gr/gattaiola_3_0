#include "rtc.h"
#include "utilities.h"
#include "sleep.h"
#include "wifi.h"
#include <cstdint>
#include "web_server.h"

// Configuration constants
const uint32_t mS_TO_S_FACTOR = 1000;  // Conversion factor for milliseconds to seconds
const uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for microseconds to seconds

const uint32_t ONE_SECOND = 1 * mS_TO_S_FACTOR;
const uint32_t UP_TIME = 30 * mS_TO_S_FACTOR;
const uint64_t SLEEP_TIME_1_MIN = 60 * uS_TO_S_FACTOR;

const uint8_t BOOT_BUTTON_PIN = 0;  // GPIO0 (usually the BOOT button)

const uint32_t LED_BUILTIN = 2;  // Most ESP32 boards have builtin LED on GPIO2

// Timing variables
unsigned long previousMillis = 0;
const unsigned long BLINK_INTERVAL = 250;  // 250ms = 2Hz (ON 250ms, OFF 250ms)

// Flag to indicate RTC error status
bool rtcError = true;

// Flag to indicate access point mode status
bool accessPointMode = false;

// LED state
bool ledState = false;

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n=== ESP32 WiFi + NTP + RTC DS3231 Sync ===");

  // Initialize builtin LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Start with LED OFF

    // Set BOOT button (usually GPIO0) as input
  pinMode(BOOT_BUTTON_PIN, INPUT);

  // Initialize I2C and RTC
  Wire.begin();
  initializeRTC();
  
  // Check RTC time validity
  rtcError != checkRTCTime();

  // Get wake up reason
  esp_sleep_wakeup_cause_t wakeup_reason = getWakeupReason();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_UNDEFINED:  // Boot after power reset
      // Read BOOT button state
      int bootButtonState = digitalRead(BOOT_BUTTON_PIN);
      Serial.print("BOOT button state: ");
      Serial.println(bootButtonState == HIGH ? "Released" : "Pressed");

      if (bootButtonState == LOW) {
        // Enter access point mode
        accessPointMode = true;

        // Initialize web server ---
        webServerSetup();
      }
    break;
  }

  // Blink built-in LED in case of error
  if (rtcError) {
    handleLEDBlink();

    if (!accessPointMode) {
      // Start the connection process
      startWiFiConnection();
    }
  }
  
  // Display final status
  displayTimeStatus();
}

void loop() {
  // Handle LED blinking (non-blocking)
  handleLEDBlink();

  // Handle WiFi state machine (non-blocking)
  //handleWiFiStateMachine();

  // Periodic status check
  //handleStatusCheck();

  // Handle web server
  webServerLoop();

  // Display time every 10 seconds
  static unsigned long lastDisplay = 0;
  static unsigned long upTime = 0;
  if (millis() - lastDisplay > ONE_SECOND) {
    displayCurrentTimes();
    lastDisplay = millis();
  }

  if ((millis() - upTime > UP_TIME) && (!rtcError)) {
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

