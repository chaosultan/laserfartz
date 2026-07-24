#include <esp_now.h>
#include <WiFi.h>

// Structure to send data to master
typedef struct struct_message {
  char deviceName[32];
  int adcValue;
  int sensorPin;
} struct_message;

// Create a struct to hold the data to send
struct_message myData;

// MAC address of master device (Homelander)
uint8_t masterAddress[] = {0x68, 0x09, 0x47, 0x58, 0xA3, 0xA8};

// Change this according to which slave device this is
const char* deviceName = "love sausage"; // Change to "puss-puss" or "lamplighter"
const int ADC_PIN = A1; // Change the pin according to your setup
const int SEND_INTERVAL = 1000; // Send data every 1 second
unsigned long lastSendTime = 0;

// Updated callback function signature
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Initialize ADC
  analogReadResolution(12); // 0-4095 for ESP32
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW callback
  esp_now_register_send_cb((esp_now_send_cb_t)OnDataSent);
  
  // Add master as a peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo)); // Important: Clear the struct
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA; // Add this line
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add master peer");
    return;
  }
  
  // Set device name
  strcpy(myData.deviceName, deviceName);
  myData.sensorPin = ADC_PIN;
  
  Serial.print("Slave device ");
  Serial.print(deviceName);
  Serial.println(" is ready!");
  Serial.print("Sending data to Homelander every ");
  Serial.print(SEND_INTERVAL/1000);
  Serial.println(" seconds");
}

void loop() {
  // Send data at the specified interval
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    // Read ADC value
    myData.adcValue = analogRead(ADC_PIN);
    
    // Send data to master
    esp_now_send(masterAddress, (uint8_t *) &myData, sizeof(myData));
    
    // Print local confirmation
    Serial.print("Sent - Device: ");
    Serial.print(deviceName);
    Serial.print(" | ADC Value: ");
    Serial.println(myData.adcValue);
    
    lastSendTime = millis();
  }
}
