#include <esp_now.h>
#include <WiFi.h>

// Structure to receive data (must match sender structure)
typedef struct struct_message {
  char deviceLabel[20];
  float temperature;
  float pressure;
  float altitude;
  int rssi;
} struct_message;

struct_message myData;

// Counter for received packets
int packetCount = 0;

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  packetCount++;
  
  // Print received data
  Serial.println("========================================");
  Serial.print("Packet #");
  Serial.println(packetCount);
  Serial.print("From MAC: ");
  for(int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if(i < 5) Serial.print(":");
  }
  Serial.println();
  
  Serial.print("Device Label: ");
  Serial.println(myData.deviceLabel);
  
  Serial.print("Temperature: ");
  Serial.print(myData.temperature, 2);
  Serial.println(" °C");
  
  Serial.print("Pressure: ");
  Serial.print(myData.pressure, 2);
  Serial.println(" hPa");
  
  Serial.print("Altitude: ");
  Serial.print(myData.altitude, 2);
  Serial.println(" m");
  
  Serial.print("RSSI (signal strength): ");
  Serial.print(myData.rssi);
  Serial.println(" dBm");
  Serial.println("========================================");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("========================================");
  Serial.println("ESP-NOW Receiver Ready");
  Serial.println("Waiting for data from 'love sausage' device...");
  Serial.println("========================================");
  Serial.println();
  
  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);
  
  // Print MAC address of this device so you can add it to the sender
  Serial.print("This device's MAC address is: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Add this MAC address to the sender code!");
  Serial.println();
}

void loop() {
  // Nothing needed here, everything is handled in the callback
  delay(1000);
}
