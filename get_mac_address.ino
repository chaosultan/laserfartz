#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Initialize Wi-Fi in Station mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(); // Wi-Fi needs to be started to get a valid MAC

  // Get and print the MAC address
  Serial.print("WiFi Station MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}
