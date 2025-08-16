#ifndef SLEEP_H
#define SLEEP_H

#include <WiFi.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

// Configuration constants
#define WAKE_PIN GPIO_NUM_0        // GPIO0 (BOOT button) for external wake
#define WAKE_PIN_LEVEL 0           // Wake when pin goes LOW (button pressed)

// Sleep duration in microseconds (1 second = 1,000,000 microseconds)
// #define SLEEP_TIME_10_SEC    10000000      // 10 seconds
// #define SLEEP_TIME_1_MIN     60000000      // 1 minute  
// #define SLEEP_TIME_5_MIN     300000000     // 5 minutes
// #define SLEEP_TIME_1_HOUR    3600000000    // 1 hour

// Wake up reasons
RTC_DATA_ATTR int bootCount = 0;  // Variable stored in RTC memory

void configureWakeSources(uint64_t sleepDuration, bool enableTimerWake, bool enableExternalWake);
void configureGPIOForSleep();
void printWakeupReason();
void displaySleepInfo(uint64_t sleepDuration, bool enableTimerWake, bool enableExternalWake);

/**
 * Enter deep sleep mode with configurable wake sources
 * @param sleepDuration Duration in microseconds (0 = sleep indefinitely)
 * @param enableTimerWake Enable timer wake up
 * @param enableExternalWake Enable external pin wake up
 */
void enterDeepSleep(uint64_t sleepDuration, bool enableTimerWake, bool enableExternalWake) {
  Serial.println("\n=== PREPARING FOR DEEP SLEEP ===");
  
  // Clean up WiFi connection to save power
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Disconnecting WiFi...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
  
  // Configure wake up sources
  configureWakeSources(sleepDuration, enableTimerWake, enableExternalWake);
  
  // Optional: Configure GPIO states to minimize power consumption
  configureGPIOForSleep();
  
  // Display sleep info
  displaySleepInfo(sleepDuration, enableTimerWake, enableExternalWake);
  
  // Final message
  Serial.println("Entering deep sleep NOW...");
  Serial.flush(); // Make sure all serial output is sent
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

/**
 * Configure wake up sources
 */
void configureWakeSources(uint64_t sleepDuration, bool enableTimerWake, bool enableExternalWake) {
  // Configure timer wake up
  if (enableTimerWake && sleepDuration > 0) {
    esp_sleep_enable_timer_wakeup(sleepDuration);
    Serial.print("Timer wake up enabled for ");
    Serial.print(sleepDuration / 1000000);
    Serial.println(" seconds");
  }
  
  // Configure external wake up (EXT0 - single pin)
  if (enableExternalWake) {
    // Enable wake up from external pin
    esp_sleep_enable_ext0_wakeup(WAKE_PIN, WAKE_PIN_LEVEL);
    
    // Configure the pin as RTC GPIO to work during sleep
    rtc_gpio_pullup_en(WAKE_PIN);
    rtc_gpio_pulldown_dis(WAKE_PIN);
    
    Serial.print("External wake up enabled on GPIO");
    Serial.print(WAKE_PIN);
    Serial.print(" (wake when ");
    Serial.print(WAKE_PIN_LEVEL ? "HIGH" : "LOW");
    Serial.println(")");
  }
}

/**
 * Configure GPIO states for minimum power consumption
 */
void configureGPIOForSleep() {
  Serial.println("Configuring GPIOs for low power...");
  
  // Isolate all GPIOs except wake pin to reduce power consumption
  // Note: This will disable all GPIOs, use carefully!
  
  // Example: Set unused pins as input with pullup to avoid floating
  for (int i = 0; i <= 39; i++) {
    if (i != WAKE_PIN) {  // Don't modify wake pin
      // Skip pins that can't be used as regular GPIO
      if (i == 6 || i == 7 || i == 8 || i == 9 || i == 10 || i == 11) {
        continue; // These are connected to flash
      }
      if (i >= 34 && i <= 39) {
        pinMode(i, INPUT); // Input only pins
      } else {
        pinMode(i, INPUT_PULLUP);
      }
    }
  }
}

/**
 * Print the reason for wake up
 */
void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  Serial.print("Wake up reason: ");
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("External signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("External signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("ULP program");
      break;
    default:
      Serial.printf("Not a deep sleep wake up: %d\n", wakeup_reason);
      break;
  }
}

/**
 * Display sleep configuration info
 */
void displaySleepInfo(uint64_t sleepDuration, bool enableTimerWake, bool enableExternalWake) {
  Serial.println("\n--- SLEEP CONFIGURATION ---");
  Serial.print("Boot count: ");
  Serial.println(bootCount);
  
  if (enableTimerWake && sleepDuration > 0) {
    Serial.print("Timer wake: ");
    Serial.print(sleepDuration / 1000000);
    Serial.println(" seconds");
  } else {
    Serial.println("Timer wake: DISABLED");
  }
  
  if (enableExternalWake) {
    Serial.print("External wake: GPIO");
    Serial.print(WAKE_PIN);
    Serial.println(" enabled");
  } else {
    Serial.println("External wake: DISABLED");
  }
  
  // Calculate estimated current consumption
  Serial.println("Estimated current in deep sleep: ~10µA");
  Serial.println("-----------------------------\n");
}

// Convenience functions for common sleep durations

/*
void sleepFor10Seconds() {
  enterDeepSleep(SLEEP_TIME_10_SEC, true, false);
}

void sleepFor1Minute() {
  enterDeepSleep(SLEEP_TIME_1_MIN, true, false);
}

void sleepFor5Minutes() {
  enterDeepSleep(SLEEP_TIME_5_MIN, true, false);
}

void sleepFor1Hour() {
  enterDeepSleep(SLEEP_TIME_1_HOUR, true, false);
}
*/

void sleepUntilButtonPress() {
  enterDeepSleep(0, false, true); // Sleep indefinitely until button press
}

void sleepWithBothWakeOptions(uint64_t duration) {
  enterDeepSleep(duration, true, true); // Both timer and button wake
}

// Function to check if we should enter sleep based on conditions
bool shouldEnterSleep() {
  // Example conditions:
  // - Battery level
  // - Time of day  
  // - Sensor readings
  // - User input
  
  // Simple example: sleep if boot count is multiple of 3
  return (bootCount % 3 == 0);
}

// Advanced: Sleep with multiple external pins (EXT1)
void sleepWithMultiplePins() {
  Serial.println("Configuring wake up from multiple pins...");
  
  // Define which pins can wake up the ESP32 (bitmask)
  uint64_t ext_wakeup_pin_1_mask = (1ULL << GPIO_NUM_0);  // GPIO0
  uint64_t ext_wakeup_pin_2_mask = (1ULL << GPIO_NUM_2);  // GPIO2
  uint64_t wakeup_pin_mask = ext_wakeup_pin_1_mask | ext_wakeup_pin_2_mask;
  
  // Configure pins as RTC GPIOs
  rtc_gpio_pullup_en(GPIO_NUM_0);
  rtc_gpio_pullup_en(GPIO_NUM_2);
  rtc_gpio_pulldown_dis(GPIO_NUM_0);
  rtc_gpio_pulldown_dis(GPIO_NUM_2);
  
  // Enable wake up from multiple pins (wake when ANY pin goes LOW)
  esp_sleep_enable_ext1_wakeup(wakeup_pin_mask, ESP_EXT1_WAKEUP_ALL_LOW);
  
  Serial.println("Wake up enabled on GPIO0 and GPIO2");
  Serial.println("ESP32 will wake when BOTH pins are LOW");
  
  Serial.flush();
  esp_deep_sleep_start();
}

#endif // SLEEP_H