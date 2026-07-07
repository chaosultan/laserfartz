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

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  // Print received data
  Serial.println("=== Received Data ===");
  Serial.print("From MAC: ");
  for(int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if(i < 5) Serial.print(":");
  }
  Serial.println();
  
  Serial.print("Device Label: ");
  Serial.println(myData.deviceLabel);
  
  Serial.print("Temperature: ");
  Serial.print(myData.temperature);
  Serial.println(" °C");
  
  Serial.print("Pressure: ");
  Serial.print(myData.pressure);
  Serial.println(" hPa");
  
  Serial.print("Altitude: ");
  Serial.print(myData.altitude);
  Serial.println(" m");
  
  Serial.print("RSSI: ");
  Serial.println(myData.rssi);
  Serial.println("====================");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  
  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("ESP-NOW Receiver Ready");
  Serial.println("Waiting for data from 'love sausage' device...");
}

void loop() {
  // Nothing needed here
}
