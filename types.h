#ifndef TYPES_H
#define TYPES_H

/**
 * Structure to store WiFi network credentials
 * Each network has SSID, password, and enabled status
 */
struct WiFiNetwork {
  String ssid;
  String password;
  bool enabled = false;
};

#endif // TYPES_H