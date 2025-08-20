#ifndef WEB_SERVER_H
#define WEB_SERVER_H

/*
 * ESP32 Web Configuration Server
 * 
 * This code creates a comprehensive web-based configuration interface for ESP32.
 * Features:
 * - WiFi network management (store up to 5 networks)
 * - Scheduled actions with configurable time
 * - Real-time system status monitoring
 * - Persistent configuration storage using Preferences
 * - Automatic WiFi connection with fallback to Access Point mode
 * - Modern, responsive web interface
 */

#include <WiFi.h>        // WiFi functionality
#include <WebServer.h>   // HTTP web server
#include <Preferences.h> // Non-volatile storage (NVS)
#include <ArduinoJson.h> // JSON parsing and generation
#include "web_page.h"
#include "types.h"

// Web server instance running on port 80
WebServer server(80);

// Preferences object for persistent storage in ESP32 flash memory
Preferences prefs;

/**
 * Main system configuration structure
 * Contains scheduled action time and WiFi networks array
 */
struct SystemConfig {
  uint8_t actionHour;        // Hour for scheduled action (0-23)
  uint8_t actionMinute;      // Minute for scheduled action (0-59)
  WiFiNetwork networks[5];   // Array of up to 5 WiFi networks
  uint8_t networkCount;      // Number of configured networks
};

// Global configuration instance
SystemConfig config;

/**
 * System time structure for basic timekeeping
 * In a real application, you would sync this with NTP server
 */
struct SystemTime {
  uint8_t hour;    // Current hour (0-23)
  uint8_t minute;  // Current minute (0-59)
  uint8_t second;  // Current second (0-59)
  uint16_t year;   // Current year
  uint8_t month;   // Current month (1-12)
  uint8_t day;     // Current day (1-31)
};

// Initialize with example time - in production, sync with NTP
SystemTime systemTime = {12, 30, 45, 2024, 8, 16};

// Function prototypes - declaration of all functions used in this program
void loadConfiguration();      // Load config from NVS memory
bool connectToSavedWiFi();    // Attempt connection to saved WiFi networks
void startAccessPoint();      // Start ESP32 as WiFi Access Point
void printServerInfo();       // Display server connection information
void updateSystemTime();      // Update internal time counter
void checkScheduledAction();  // Check if scheduled action should execute
void executeScheduledAction(); // Execute the scheduled action
void handleRoot();
void handleGetStatus();       // API: Get system status
void handleGetConfig();       // API: Get configuration
void handleSetConfig();       // API: Save configuration
void handleGetNetworks();     // API: Get WiFi networks
void handleSetNetworks();     // API: Save WiFi networks
void handleGetTime();         // API: Get current time
void handleNotFound();        // Handle 404 errors
void saveNetworksToPrefs();   // Save networks to persistent storage
void webServerSetup();
void webServerLoop();

/**
 * Configure all web server routes and their corresponding handler functions
 * Sets up both the main HTML page and all API endpoints
 */
void setupWebServer() {
  // Main page - serves the HTML interface
  server.on("/", HTTP_GET, handleRoot);
  
  // REST API endpoints for system interaction
  server.on("/api/status", HTTP_GET, handleGetStatus);     // GET system status
  server.on("/api/config", HTTP_GET, handleGetConfig);     // GET configuration
  server.on("/api/config", HTTP_POST, handleSetConfig);    // POST save configuration
  server.on("/api/networks", HTTP_GET, handleGetNetworks); // GET WiFi networks
  server.on("/api/networks", HTTP_POST, handleSetNetworks);// POST save WiFi networks
  server.on("/api/time", HTTP_GET, handleGetTime);         // GET current time
  
  // Handle requests to non-existent pages
  server.onNotFound(handleNotFound);
}

/**
 * Serve the main HTML page with embedded CSS and JavaScript
 * This creates a complete single-page application
 */
void handleRoot() {  
  // Send the complete HTML page to the client
  server.send(200, "text/html", html);
}

/**
 * API Endpoint: GET /api/status
 * Returns current system status including WiFi, IP, uptime, memory, and time
 */
void handleGetStatus() {
  JsonDocument doc;
  
  // WiFi connection status
  doc["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
  // Current IP address (empty string if not connected)
  doc["ipAddress"] = WiFi.localIP().toString();
  // System uptime in seconds since boot
  doc["uptime"] = millis() / 1000;
  // Available heap memory in bytes
  doc["freeHeap"] = ESP.getFreeHeap();
  
  // Create nested object for system time
  JsonObject timeObj = doc["systemTime"].to<JsonObject>();
  timeObj["hour"] = systemTime.hour;
  timeObj["minute"] = systemTime.minute;
  timeObj["second"] = systemTime.second;
  timeObj["year"] = systemTime.year;
  timeObj["month"] = systemTime.month;
  timeObj["day"] = systemTime.day;
  
  // Convert JSON to string and send response
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

/**
 * API Endpoint: GET /api/config
 * Returns the current scheduled action configuration
 */
void handleGetConfig() {
  JsonDocument doc;
  
  // Current scheduled action time
  doc["actionHour"] = config.actionHour;
  doc["actionMinute"] = config.actionMinute;
  
  // Convert to JSON and send
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

/**
 * API Endpoint: POST /api/config
 * Updates the scheduled action configuration and saves to persistent storage
 */
void handleSetConfig() {
  // Check if request contains JSON data
  if (server.hasArg("plain")) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    // Parse JSON successfully
    if (!error) {
      // Update configuration with new values (use defaults if not provided)
      config.actionHour = doc["actionHour"] | 12;
      config.actionMinute = doc["actionMinute"] | 30;
      
      // Save configuration to persistent storage (NVS)
      prefs.putUChar("actionHour", config.actionHour);
      prefs.putUChar("actionMinute", config.actionMinute);
      
      // Send success response
      server.send(200, "application/json", "{\"success\":true}");
      
      // Log the update to serial console
      Serial.print("Scheduled action time updated: ");
      Serial.print(config.actionHour);
      Serial.print(":");
      Serial.println(config.actionMinute);
    } else {
      // JSON parsing failed
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
    }
  } else {
    // No data received
    server.send(400, "application/json", "{\"success\":false,\"error\":\"No data\"}");
  }
}

/**
 * API Endpoint: GET /api/networks
 * Returns all configured WiFi networks (passwords included for editing)
 */
void handleGetNetworks() {
  JsonDocument doc;
  JsonArray networksArray = doc["networks"].to<JsonArray>();
  
  // Add all configured networks to the response
  for (uint8_t i = 0; i < config.networkCount && i < 5; i++) {
    JsonObject network = networksArray.add<JsonObject>();
    network["ssid"] = config.networks[i].ssid;
    network["password"] = config.networks[i].password;
    network["enabled"] = config.networks[i].enabled;
  }
  
  // Send the networks array
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

/**
 * API Endpoint: POST /api/networks
 * Updates the WiFi networks configuration and saves to persistent storage
 */
void handleSetNetworks() {
  // Check if request contains JSON data
  if (server.hasArg("plain")) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    // Parse JSON successfully
    if (!error) {
      JsonArray networks = doc["networks"];
      
      // Clear existing networks configuration
      config.networkCount = 0;
      
      // Process up to 5 networks from the request
      for (uint8_t i = 0; i < networks.size() && i < 5; i++) {
        JsonObject network = networks[i];
        config.networks[i].ssid = network["ssid"].as<String>();
        config.networks[i].password = network["password"].as<String>();
        config.networks[i].enabled = network["enabled"] | false;
        config.networkCount++;
      }
      
      // Save networks to persistent storage
      saveNetworksToPrefs();
      
      // Send success response
      server.send(200, "application/json", "{\"success\":true}");
      
      // Log the networks update to serial console
      Serial.println("WiFi networks updated:");
      for (uint8_t i = 0; i < config.networkCount; i++) {
        Serial.print("  ");
        Serial.print(i + 1);
        Serial.print(". ");
        Serial.print(config.networks[i].ssid);
        Serial.print(" (");
        Serial.print(config.networks[i].enabled ? "enabled" : "disabled");
        Serial.println(")");
      }
    } else {
      // JSON parsing failed
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
    }
  } else {
    // No data received
    server.send(400, "application/json", "{\"success\":false,\"error\":\"No data\"}");
  }
}

/**
 * API Endpoint: GET /api/time
 * Returns current system time as JSON object
 */
void handleGetTime() {
  JsonDocument doc;
  
  // Current system time values
  doc["hour"] = systemTime.hour;
  doc["minute"] = systemTime.minute;
  doc["second"] = systemTime.second;
  doc["year"] = systemTime.year;
  doc["month"] = systemTime.month;
  doc["day"] = systemTime.day;
  
  // Send time data
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

/**
 * Handle requests to non-existent pages (404 errors)
 */
void handleNotFound() {
  server.send(404, "text/plain", "Page not found");
}

/**
 * Load system configuration from persistent storage (NVS)
 * Called once during startup to restore saved settings
 */
void loadConfiguration() {
  Serial.println("Loading configuration from memory...");
  
  // Load scheduled action time with default values if not found
  config.actionHour = prefs.getUChar("actionHour", 12);
  config.actionMinute = prefs.getUChar("actionMinute", 30);
  
  // Load number of configured WiFi networks
  config.networkCount = prefs.getUChar("networkCount", 0);
  
  // Load each WiFi network configuration
  for (uint8_t i = 0; i < config.networkCount && i < 5; i++) {
    // Create unique keys for each network's data
    String ssidKey = "ssid" + String(i);
    String passKey = "pass" + String(i);
    String enabledKey = "enabled" + String(i);
    
    // Load network data with empty defaults
    config.networks[i].ssid = prefs.getString(ssidKey.c_str(), "");
    config.networks[i].password = prefs.getString(passKey.c_str(), "");
    config.networks[i].enabled = prefs.getBool(enabledKey.c_str(), false);

    // Print loaded network info to serial
    Serial.print("Network ");
    Serial.print(i + 1);
    Serial.print(": SSID='");
    Serial.print(config.networks[i].ssid);
    Serial.print("' Password='");
    Serial.print(config.networks[i].password);
    Serial.print("' Enabled=");
    Serial.println(config.networks[i].enabled ? "true" : "false");
  }
  
  // Log loaded configuration to serial console
  Serial.print("Scheduled action: ");
  Serial.print(config.actionHour);
  Serial.print(":");
  Serial.println(config.actionMinute);
  Serial.print("Loaded ");
  Serial.print(config.networkCount);
  Serial.println(" WiFi networks");
}

/**
 * Save WiFi networks configuration to persistent storage (NVS)
 * Called whenever networks are updated via web interface
 */
void saveNetworksToPrefs() {
  // Save the number of networks
  prefs.putUChar("networkCount", config.networkCount);
  
  // Save each network's data with unique keys
  for (uint8_t i = 0; i < config.networkCount; i++) {
    String ssidKey = "ssid" + String(i);
    String passKey = "pass" + String(i);
    String enabledKey = "enabled" + String(i);
    
    // Store SSID, password, and enabled status
    prefs.putString(ssidKey.c_str(), config.networks[i].ssid);
    prefs.putString(passKey.c_str(), config.networks[i].password);
    prefs.putBool(enabledKey.c_str(), config.networks[i].enabled);
  }
}

/**
 * Attempt to connect to WiFi using saved network configurations
 * Tries each enabled network in order until connection succeeds
 * @return true if connection successful, false if all networks failed
 */
bool connectToSavedWiFi() {
  Serial.println("Trying to connect to saved WiFi networks...");
  
  // Try each configured network
  for (uint8_t i = 0; i < config.networkCount; i++) {
    // Only try enabled networks with valid SSID
    if (config.networks[i].enabled && config.networks[i].ssid.length() > 0) {
      Serial.print("Connecting to: ");
      Serial.println(config.networks[i].ssid);
      
      // Start WiFi connection attempt
      WiFi.begin(config.networks[i].ssid.c_str(), config.networks[i].password.c_str());
      
      // Wait up to 10 seconds for connection (20 attempts * 500ms)
      uint8_t attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      
      // Check if connection was successful
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ Connected to WiFi!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true; // Success - exit function
      } else {
        Serial.println("\n✗ Failed");
        WiFi.disconnect(); // Clean up failed connection
      }
    }
  }
  
  // All networks failed or no networks configured
  return false;
}

/**
 * Start ESP32 as WiFi Access Point for initial configuration
 * Creates a hotspot that users can connect to for setup
 */
void startAccessPoint() {
  Serial.println("Starting Access Point mode...");
  
  // Configure ESP32 as Access Point
  WiFi.mode(WIFI_AP);
  // Create hotspot with SSID "ESP32-Config" and password "12345678"
  WiFi.softAP("ESP32-Config", "12345678");
  
  // Display Access Point IP address
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP());
}

/**
 * Display web server connection information to serial console
 * Shows different info depending on WiFi mode (client vs AP)
 */
void printServerInfo() {
  Serial.println("\n=== Web Server Info ===");
  // Check current WiFi mode
  if (WiFi.getMode() == WIFI_AP) {
    // Access Point mode - show hotspot info
    Serial.println("Mode: Access Point");
    Serial.print("SSID: ESP32-Config");
    Serial.println(" | Password: 12345678");
    Serial.print("Visit: http://");
    Serial.println(WiFi.softAPIP());
  } else {
    // Client mode - show network info
    Serial.println("Mode: WiFi Client");
    Serial.print("Visit: http://");
    Serial.println(WiFi.localIP());
  }
  Serial.println("======================\n");
}

/**
 * Update internal system time counter
 * Simple time simulation that increments every second
 * In production, you would sync with NTP server instead
 */
void updateSystemTime() {
  static uint32_t lastUpdate = 0;
  
  // Update time every 1000ms (1 second)
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    
    // Increment seconds and handle rollovers
    systemTime.second++;
    if (systemTime.second >= 60) {
      systemTime.second = 0;
      systemTime.minute++;
      if (systemTime.minute >= 60) {
        systemTime.minute = 0;
        systemTime.hour++;
        if (systemTime.hour >= 24) {
          systemTime.hour = 0;
          // In a complete implementation, you would also increment day/month/year
        }
      }
    }
  }
}

/**
 * Check if it's time to execute the scheduled action
 * Compares current time with configured action time
 * Ensures action only executes once per day
 */
void checkScheduledAction() {
  static bool actionExecutedToday = false;
  
  // Reset the daily execution flag at midnight (00:00)
  if (systemTime.hour == 0 && systemTime.minute == 0) {
    actionExecutedToday = false;
  }
  
  // Check if current time matches scheduled time and hasn't been executed today
  if (!actionExecutedToday && 
      systemTime.hour == config.actionHour && 
      systemTime.minute == config.actionMinute) {
    
    // Execute the scheduled action
    executeScheduledAction();
    actionExecutedToday = true; // Prevent multiple executions
  }
}

/**
 * Execute the scheduled action
 * This is where you implement your custom scheduled functionality
 * Examples: turn on/off relays, send notifications, collect sensor data, etc.
 */
void executeScheduledAction() {
  Serial.println("\n🎯 EXECUTING SCHEDULED ACTION!");
  Serial.print("Time: ");
  Serial.print(systemTime.hour);
  Serial.print(":");
  Serial.println(systemTime.minute);
  
  // *** ADD YOUR CUSTOM SCHEDULED ACTION CODE HERE ***
  // Examples:
  // - digitalWrite(RELAY_PIN, HIGH);  // Turn on a relay
  // - sendHttpRequest();              // Send data to web service
  // - readSensors();                  // Collect sensor data
  // - sendNotification();             // Send push notification
  
  Serial.println("Scheduled action completed!\n");
}

void webServerSetup() {
  // Initialize the Preferences library for persistent storage
  // "esp32-config" is the namespace, false means read/write access
  prefs.begin("esp32-config", false);

  // Load previously saved configuration from flash memory
  loadConfiguration();

  // Configure all web server routes and endpoints
  setupWebServer();

  // Start the HTTP server
  server.begin();
  Serial.println("Web server started!");
  printServerInfo();
}

void webServerLoop() {
  // Process incoming HTTP requests
  server.handleClient();

  // Update internal time counter (increments every second)
  updateSystemTime();

  // Check if it's time to execute the scheduled action
  checkScheduledAction();
}

#endif // WEB_SERVER_H