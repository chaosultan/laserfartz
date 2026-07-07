#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// BMP280 I2C pins for ESP-C3
#define I2C_SDA 8
#define I2C_SCL 9

// Replace with your main device's MAC address
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

// ESP-NOW callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C with custom pins
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize BMP280
  if (!bmp.begin(0x76)) {  // Try 0x76 address first, if not working try 0x77
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  
  // Configure BMP280
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temperature oversampling
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,      // Filtering
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time
  
  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  
  // Register send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer (main device)
  esp_now_add_peer(mainDeviceMac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  
  // Set device label
  strcpy(myData.deviceLabel, "love sausage");
  
  Serial.println("ESP-C3 ESP-NOW Sender with BMP280");
  Serial.println("Device Label: love sausage");
  Serial.println("Ready to send data!");
}

void loop() {
  // Read sensor data
  myData.temperature = bmp.readTemperature();
  myData.pressure = bmp.readPressure() / 100.0F; // Convert to hPa
  myData.altitude = bmp.readAltitude(1013.25); // Sea level pressure in hPa
  myData.rssi = WiFi.RSSI();
  
  // Print to serial monitor for debugging
  Serial.println("=== Sending Data ===");
  Serial.print("Device: ");
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
  
  // Send data via ESP-NOW
  esp_now_send(mainDeviceMac, (uint8_t *) &myData, sizeof(myData));
  
  // Wait before next reading
  delay(5000); // Send every 5 seconds
}
