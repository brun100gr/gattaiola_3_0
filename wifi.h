#ifndef WIFI_H
#define WIFI_H

#include "secrets.h"

const uint8_t NUM_NETWORKS = sizeof(networks) / sizeof(networks[0]);

// WiFi Connection State Machine
enum WiFiState {
  WIFI_DISCONNECTED,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_RECONNECTING
};

// Global variables
WiFiState currentWiFiState = WIFI_DISCONNECTED;
uint8_t currentNetworkIndex = 0;
uint8_t connectionAttempts = 0;
uint32_t lastConnectionAttempt = 0;
uint32_t lastStatusCheck = 0;
uint32_t connectionStartTime = 0;

// Configuration constants
const uint8_t MAX_ATTEMPTS_PER_NETWORK = 3;      // Attempts per network before moving to next
const uint32_t CONNECTION_TIMEOUT = 10000;       // 10 seconds timeout per attempt
const uint32_t ATTEMPT_DELAY = 2000;             // 2 seconds between attempts
const uint32_t STATUS_CHECK_INTERVAL = 5000;     // Check connection every 5 seconds
const uint32_t RECONNECT_DELAY = 3000;           // Wait 3 seconds before reconnecting

void handleWiFiStateMachine();
void startWiFiConnection();
void attemptConnection();
void onConnectionSuccess();
void onConnectionTimeout();
void onConnectionLost();
void moveToNextNetwork();
void handleStatusCheck();
String getStateString(WiFiState state);
void forceReconnection();
void printConnectionInfo();

void handleWiFiStateMachine() {
  uint32_t currentMillis = millis();
  
  switch (currentWiFiState) {
    case WIFI_DISCONNECTED:
      // Try to connect to current network
      if (currentMillis - lastConnectionAttempt >= ATTEMPT_DELAY) {
        attemptConnection();
      }
      break;
      
    case WIFI_CONNECTING:
      // Check if connection succeeded or timed out
      if (WiFi.status() == WL_CONNECTED) {
        onConnectionSuccess();
      } else if (currentMillis - connectionStartTime >= CONNECTION_TIMEOUT) {
        onConnectionTimeout();
      }
      break;
      
    case WIFI_CONNECTED:
      // Check if connection is still alive
      if (WiFi.status() != WL_CONNECTED) {
        onConnectionLost();
      }
      break;
      
    case WIFI_RECONNECTING:
      // Wait before attempting reconnection
      if (currentMillis - lastConnectionAttempt >= RECONNECT_DELAY) {
        Serial.println("Attempting to reconnect to last known network...");
        currentWiFiState = WIFI_DISCONNECTED;
      }
      break;
  }
}

void startWiFiConnection() {
  Serial.println("Starting WiFi connection process...");
  currentWiFiState = WIFI_DISCONNECTED;
  currentNetworkIndex = 0;
  connectionAttempts = 0;
  lastConnectionAttempt = 0;
}

void attemptConnection() {
  if (connectionAttempts >= MAX_ATTEMPTS_PER_NETWORK) {
    // Move to next network
    moveToNextNetwork();
    return;
  }
  
  connectionAttempts++;
  lastConnectionAttempt = millis();
  connectionStartTime = millis();
  
  Serial.print("Attempting to connect to '");
  Serial.print(networks[currentNetworkIndex].ssid);
  Serial.print("' (attempt ");
  Serial.print(connectionAttempts);
  Serial.print("/");
  Serial.print(MAX_ATTEMPTS_PER_NETWORK);
  Serial.println(")");
  
  // Start connection attempt
  WiFi.mode(WIFI_STA);
  WiFi.begin(networks[currentNetworkIndex].ssid, networks[currentNetworkIndex].password);
  
  currentWiFiState = WIFI_CONNECTING;
}

void onConnectionSuccess() {
  currentWiFiState = WIFI_CONNECTED;
  
  Serial.println("\n✓ WiFi Connected!");
  Serial.print("Connected to: ");
  Serial.println(networks[currentNetworkIndex].ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("Total attempts needed: ");
  Serial.println(connectionAttempts);
  Serial.println();
  
  // Reset attempt counter for future use
  connectionAttempts = 0;
}

void onConnectionTimeout() {
  Serial.print("✗ Connection timeout for '");
  Serial.print(networks[currentNetworkIndex].ssid);
  Serial.println("'");
  
  WiFi.disconnect();
  currentWiFiState = WIFI_DISCONNECTED;
}

void onConnectionLost() {
  Serial.println("⚠ WiFi connection lost!");
  Serial.print("Last connected to: ");
  Serial.println(networks[currentNetworkIndex].ssid);
  
  WiFi.disconnect();
  currentWiFiState = WIFI_RECONNECTING;
  lastConnectionAttempt = millis();
  connectionAttempts = 0; // Reset attempts for reconnection
}

void moveToNextNetwork() {
  Serial.print("Moving to next network after ");
  Serial.print(MAX_ATTEMPTS_PER_NETWORK);
  Serial.println(" failed attempts");
  
  // Move to next network in the list
  currentNetworkIndex = (currentNetworkIndex + 1) % NUM_NETWORKS;
  connectionAttempts = 0;
  
  Serial.print("Now trying network ");
  Serial.print(currentNetworkIndex + 1);
  Serial.print("/");
  Serial.print(NUM_NETWORKS);
  Serial.print(": '");
  Serial.print(networks[currentNetworkIndex].ssid);
  Serial.println("'");
  
  // Small delay before trying next network
  lastConnectionAttempt = millis();
}

void handleStatusCheck() {
  uint32_t currentMillis = millis();
  
  if (currentMillis - lastStatusCheck >= STATUS_CHECK_INTERVAL) {
    lastStatusCheck = currentMillis;
    
    Serial.println("--- WiFi Status ---");
    Serial.print("State: ");
    Serial.println(getStateString(currentWiFiState));
    Serial.print("Current network: ");
    Serial.print(networks[currentNetworkIndex].ssid);
    Serial.print(" (");
    Serial.print(currentNetworkIndex + 1);
    Serial.print("/");
    Serial.print(NUM_NETWORKS);
    Serial.println(")");
    
    if (currentWiFiState == WIFI_CONNECTED) {
      Serial.print("IP: ");
      Serial.print(WiFi.localIP());
      Serial.print(" | RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
    } else {
      Serial.print("Connection attempts: ");
      Serial.print(connectionAttempts);
      Serial.print("/");
      Serial.println(MAX_ATTEMPTS_PER_NETWORK);
    }
    
    Serial.print("Uptime: ");
    Serial.print(currentMillis / 1000);
    Serial.println(" seconds");
    Serial.println();
  }
}

// Utility functions
String getStateString(WiFiState state) {
  switch (state) {
    case WIFI_DISCONNECTED: return "DISCONNECTED";
    case WIFI_CONNECTING: return "CONNECTING";
    case WIFI_CONNECTED: return "CONNECTED";
    case WIFI_RECONNECTING: return "RECONNECTING";
    default: return "UNKNOWN";
  }
}

// Function to manually trigger reconnection (useful for testing)
void forceReconnection() {
  Serial.println("Forcing WiFi reconnection...");
  WiFi.disconnect();
  currentWiFiState = WIFI_DISCONNECTED;
  connectionAttempts = 0;
  lastConnectionAttempt = 0;
}

// Function to get current connection info
void printConnectionInfo() {
  if (currentWiFiState == WIFI_CONNECTED) {
    Serial.println("=== Current Connection Info ===");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("Not connected to WiFi");
  }
}

#endif // WIFI_H