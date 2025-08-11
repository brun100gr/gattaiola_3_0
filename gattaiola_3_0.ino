#include <WiFi.h>

// Your WiFi credentials
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// Variables to track WiFi connection
unsigned long wifiStartTime = 0;        // When we started trying to connect
const unsigned long wifiTimeout = 10000; // Max time to try (ms)
bool wifiConnecting = false;            // Are we currently trying to connect?

// This is another example "task" that will run in loop()
// while WiFi is trying to connect
unsigned long lastBlinkTime = 0;
bool ledState = false;
const int ledPin = 2; // Built-in LED on many ESP32 boards

void startWiFiConnection() {
  Serial.println("Starting WiFi connection...");
  WiFi.mode(WIFI_STA); // Station mode (client)
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  wifiStartTime = millis();
  wifiConnecting = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Start WiFi connection process (non-blocking)
  startWiFiConnection();
}

void loop() {
  // ------------------------------
  // 1. Handle WiFi connection state
  // ------------------------------
  if (wifiConnecting) {
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
      Serial.print("Connected to WiFi! IP address: ");
      Serial.println(WiFi.localIP());
      wifiConnecting = false; // Done connecting
    }
    else if (millis() - wifiStartTime > wifiTimeout) {
      Serial.println("WiFi connection attempt timed out.");
      wifiConnecting = false;
      // Here you can retry, connect to another network, or switch to offline mode
    }
    else {
      // Still trying...
      // This runs without blocking
      static unsigned long lastStatusPrint = 0;
      if (millis() - lastStatusPrint > 1000) {
        Serial.println("Still connecting to WiFi...");
        lastStatusPrint = millis();
      }
    }
  }

  // ------------------------------
  // 2. Example of another task running in parallel
  // ------------------------------
  if (millis() - lastBlinkTime > 500) { // Every 500ms
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    lastBlinkTime = millis();
  }

  // ------------------------------
  // 3. You can put more code here
  // ------------------------------
  // For example, sensor reading, serial commands, etc.
}
