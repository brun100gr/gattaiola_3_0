#include <WiFi.h>

// ===============================
// WiFi network list
// ===============================
// You can add as many as you want here
struct WiFiNetwork {
  const char* ssid;
  const char* password;
};

WiFiNetwork networks[] = {
  { "WiFiHome", "HomePassword" },
  { "WiFiOffice", "OfficePassword" },
  { "WiFiHotspot", "HotspotPassword" }
};

const int networkCount = sizeof(networks) / sizeof(networks[0]);

// ===============================
// Connection variables
// ===============================
int currentNetworkIndex = 0;
bool wifiConnecting = false;
unsigned long wifiStartTime = 0;
const unsigned long wifiTimeout = 8000; // milliseconds

// ===============================
// Example parallel task variables
// ===============================
unsigned long lastBlinkTime = 0;
bool ledState = false;
const int ledPin = 2; // Built-in LED pin for ESP32

// ===============================
// Start connection to the current network
// ===============================
void startWiFiConnection() {
  Serial.print("Trying to connect to: ");
  Serial.println(networks[currentNetworkIndex].ssid);

  WiFi.mode(WIFI_STA); // Station mode
  WiFi.begin(networks[currentNetworkIndex].ssid, networks[currentNetworkIndex].password);
  
  wifiStartTime = millis();
  wifiConnecting = true;
}

// ===============================
// Try the next network in the list
// ===============================
void tryNextNetwork() {
  currentNetworkIndex++;
  if (currentNetworkIndex >= networkCount) {
    Serial.println("No more networks to try. Going offline.");
    wifiConnecting = false;
    return;
  }
  startWiFiConnection();
}

// ===============================
// Setup
// ===============================
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Start with the first network
  startWiFiConnection();
}

// ===============================
// Main loop
// ===============================
void loop() {
  // 1. Handle WiFi connection process
  if (wifiConnecting) {
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
      Serial.print("Connected to ");
      Serial.print(networks[currentNetworkIndex].ssid);
      Serial.print(" | IP: ");
      Serial.println(WiFi.localIP());
      wifiConnecting = false; // Stop connection attempts
    }
    else if (millis() - wifiStartTime > wifiTimeout) {
      Serial.print("Failed to connect to: ");
      Serial.println(networks[currentNetworkIndex].ssid);
      wifiConnecting = false;
      tryNextNetwork(); // Move to next network
    }
    else {
      // Still trying
      static unsigned long lastStatusPrint = 0;
      if (millis() - lastStatusPrint > 1000) {
        Serial.println("Connecting...");
        lastStatusPrint = millis();
      }
    }
  }

  // 2. Example of another task (LED blink)
  if (millis() - lastBlinkTime > 500) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    lastBlinkTime = millis();
  }

  // 3. You can add other non-blocking tasks here
}
