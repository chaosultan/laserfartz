#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// BMP280 I2C pins for ESP-C3
#define I2C_SDA 8
#define I2C_SCL 9

// Replace with your main device's MAC address
// Get this by running the MAC address code on your main device
uint8_t mainDeviceMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // CHANGE THIS!

// Structure to send data
typedef struct struct_message {
  char deviceLabel[20];
  float temperature;
  float pressure;
  float altitude;
  int rssi;
} struct_message;

struct_message myData;

// BMP280 object
Adafruit_BMP280 bmp;

// ESP-NOW callback when data is sent - CORRECTED SIGNATURE for your core
// Using wifi_tx_info_t* instead of const uint8_t*
void OnDataSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("Starting ESP-C3 ESP-NOW Sender...");
  
  // Initialize I2C with custom pins
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize BMP280
  Serial.println("Initializing BMP280 sensor...");
  if (!bmp.begin(0x76)) {  // Try 0x76 address first
    Serial.println("Could not find BMP280 at 0x76, trying 0x77...");
    if (!bmp.begin(0x77)) {
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
      while (1) {
        delay(1000);
        Serial.println("BMP280 not found! Please check connections.");
      }
    }
  }
  
  // Configure BMP280
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temperature oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time
  
  Serial.println("BMP280 initialized successfully!");
  
  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  Serial.println("ESP-NOW initialized successfully!");
  
  // Register send callback with correct signature
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer (main device)
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));  // Zero out the structure
  memcpy(peerInfo.peer_addr, mainDeviceMac, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer - check MAC address!");
    Serial.println("Make sure the MAC address is correct and the main device is powered on.");
    return;
  }
  
  Serial.println("Peer added successfully!");
  
  // Set device label
  strcpy(myData.deviceLabel, "love sausage");
  
  Serial.println("=========================================");
  Serial.println("ESP-C3 ESP-NOW Sender with BMP280");
  Serial.print("Device Label: ");
  Serial.println(myData.deviceLabel);
  Serial.print("Sending to MAC: ");
  for(int i = 0; i < 6; i++) {
    Serial.printf("%02X", mainDeviceMac[i]);
    if(i < 5) Serial.print(":");
  }
  Serial.println();
  Serial.println("Ready to send data every 5 seconds!");
  Serial.println("=========================================");
}

void loop() {
  // Read sensor data
  float temp = bmp.readTemperature();
  float press = bmp.readPressure() / 100.0F; // Convert to hPa
  float alt = bmp.readAltitude(1013.25); // Sea level pressure in hPa
  int rssi = WiFi.RSSI();
  
  // Check if readings are valid
  if (isnan(temp) || isnan(press)) {
    Serial.println("Failed to read from BMP280 sensor!");
    delay(5000);
    return;
  }
  
  // Update data structure
  myData.temperature = temp;
  myData.pressure = press;
  myData.altitude = alt;
  myData.rssi = rssi;
  
  // Print to serial monitor for debugging
  Serial.println("=== Sending Data ===");
  Serial.print("Device: ");
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
  Serial.print("RSSI: ");
  Serial.println(myData.rssi);
  Serial.println("====================");
  
  // Send data via ESP-NOW
  esp_err_t result = esp_now_send(mainDeviceMac, (uint8_t *) &myData, sizeof(myData));
  
  if (result == ESP_OK) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.print("Error sending data: ");
    Serial.println(result);
  }
  
  Serial.println();
  
  // Wait before next reading
  delay(5000); // Send every 5 seconds
}
