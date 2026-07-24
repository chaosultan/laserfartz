#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// Structure to receive data
typedef struct struct_message {
  char deviceName[32];
  int adcValue;
  int sensorPin;
} struct_message;

struct_message loveSausageData;
struct_message pussPussData;
struct_message lamplighterData;

bool loveSausageReceived = false;
bool pussPussReceived = false;
bool lamplighterReceived = false;

// MAC addresses (REPLACE WITH YOUR ACTUAL MAC ADDRESSES)
uint8_t loveSausageAddress[] = {0xE8, 0x3D, 0xC1, 0x8E, 0x3D, 0xA4};
uint8_t pussPussAddress[] = {0xE8, 0x3D, 0xC1, 0x8E, 0x3D, 0xA5};
uint8_t lamplighterAddress[] = {0xE8, 0x3D, 0xC1, 0x8E, 0x3D, 0xA6};

// Colors
#define COLOR_LOVE_SAUSAGE tft.color565(255, 80, 80)    // Red
#define COLOR_PUSS_PUSS tft.color565(200, 50, 255)      // Purple
#define COLOR_LAMPLIGHTER tft.color565(80, 255, 80)     // Green
#define COLOR_BG TFT_BLACK

unsigned long lastDisplayUpdate = 0;

// ============ FUNCTION PROTOTYPES (DECLARATIONS) ============
// These tell the compiler that these functions exist and will be defined later
void drawDeviceSection(int x, int y, int width, int height, String name, uint16_t color, bool received);
void updateStatusBar();
void updateDisplay();
void drawLayout();
// ===========================================================

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  struct_message receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  
  if (memcmp(mac, loveSausageAddress, 6) == 0) {
    loveSausageData = receivedData;
    loveSausageReceived = true;
    Serial.println("Received from Love Sausage");
  } 
  else if (memcmp(mac, pussPussAddress, 6) == 0) {
    pussPussData = receivedData;
    pussPussReceived = true;
    Serial.println("Received from Puss-Puss");
  } 
  else if (memcmp(mac, lamplighterAddress, 6) == 0) {
    lamplighterData = receivedData;
    lamplighterReceived = true;
    Serial.println("Received from Lamplighter");
  }
  
  // Update display immediately when data is received
  updateDisplay();
}

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(1); // Landscape mode, adjust as needed
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(TFT_WHITE, COLOR_BG);
  
  // Draw initial layout
  drawLayout();
  
  // Setup ESP-NOW
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    tft.setCursor(10, 100);
    tft.setTextColor(TFT_RED);
    tft.println("ESP-NOW ERROR!");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register peers
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  
  // Add all three peers
  esp_now_peer_info_t peers[3] = {};
  
  memcpy(peers[0].peer_addr, loveSausageAddress, 6);
  peers[0].channel = 0;
  peers[0].encrypt = false;
  peers[0].ifidx = WIFI_IF_STA;
  
  memcpy(peers[1].peer_addr, pussPussAddress, 6);
  peers[1].channel = 0;
  peers[1].encrypt = false;
  peers[1].ifidx = WIFI_IF_STA;
  
  memcpy(peers[2].peer_addr, lamplighterAddress, 6);
  peers[2].channel = 0;
  peers[2].encrypt = false;
  peers[2].ifidx = WIFI_IF_STA;
  
  for (int i = 0; i < 3; i++) {
    if (esp_now_add_peer(&peers[i]) != ESP_OK) {
      Serial.print("Failed to add peer ");
      Serial.println(i);
    }
  }
  
  Serial.println("Homelander ready!");
  Serial.println("Screen width: " + String(tft.width()));
  Serial.println("Screen height: " + String(tft.height()));
}

void loop() {
  // Update display periodically even if no data is received
  if (millis() - lastDisplayUpdate > 5000) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  delay(100);
}

// ============ FUNCTION DEFINITIONS ============

void drawLayout() {
  tft.fillScreen(COLOR_BG);
  
  // Title
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 5);
  tft.println("HOMELANDER");
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(10, 30);
  tft.println("ESP-NOW Receiver");
  
  // Separator
  tft.drawLine(0, 45, tft.width(), 45, TFT_BLUE);
  
  // Calculate section width based on screen width
  int screenWidth = tft.width();
  int sectionWidth = (screenWidth - 40) / 3;
  int sectionX1 = 10;
  int sectionX2 = 20 + sectionWidth;
  int sectionX3 = 30 + (sectionWidth * 2);
  
  // Draw sections for each device
  drawDeviceSection(sectionX1, 55, sectionWidth, 180, "Love Sausage", COLOR_LOVE_SAUSAGE, loveSausageReceived);
  drawDeviceSection(sectionX2, 55, sectionWidth, 180, "Puss-Puss", COLOR_PUSS_PUSS, pussPussReceived);
  drawDeviceSection(sectionX3, 55, sectionWidth, 180, "Lamplighter", COLOR_LAMPLIGHTER, lamplighterReceived);
  
  // Status bar
  int statusY = tft.height() - 20;
  tft.drawLine(0, statusY, tft.width(), statusY, TFT_BLUE);
  updateStatusBar();
}

void drawDeviceSection(int x, int y, int width, int height, String name, uint16_t color, bool received) {
  // Background
  tft.fillRect(x, y, width, height, tft.color565(20, 20, 20));
  
  // Border
  tft.drawRect(x, y, width, height, color);
  
  // Device name
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.setCursor(x + 5, y + 5);
  tft.println(name);
  
  // Separator
  tft.drawLine(x + 2, y + 22, x + width - 2, y + 22, TFT_DARKGREY);
  
  if (received) {
    // Status indicator
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(x + 5, y + 28);
    tft.print("● ONLINE");
    
    // Get the appropriate value
    int adcValue = 0;
    if (name == "Love Sausage") adcValue = loveSausageData.adcValue;
    else if (name == "Puss-Puss") adcValue = pussPussData.adcValue;
    else if (name == "Lamplighter") adcValue = lamplighterData.adcValue;
    
    // Display value in large font
    tft.setTextColor(color);
    tft.setTextSize(3);
    char valueStr[10];
    sprintf(valueStr, "%4d", adcValue);
    tft.setCursor(x + 10, y + 50);
    tft.println(valueStr);
    
    // Draw bar graph (0-4095 scale)
    int barWidth = map(adcValue, 0, 4095, 0, width - 20);
    if (barWidth < 0) barWidth = 0;
    if (barWidth > width - 20) barWidth = width - 20;
    
    tft.fillRect(x + 10, y + 110, barWidth, 15, color);
    tft.drawRect(x + 10, y + 110, width - 20, 15, TFT_DARKGREY);
    
    // Show percentage
    int percentage = map(adcValue, 0, 4095, 0, 100);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(x + 10, y + 135);
    tft.print(percentage);
    tft.print("%");
    
    // Pin info
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor(x + 10, y + 155);
    tft.print("Pin: A");
    if (name == "Love Sausage") tft.print(loveSausageData.sensorPin);
    else if (name == "Puss-Puss") tft.print(pussPussData.sensorPin);
    else if (name == "Lamplighter") tft.print(lamplighterData.sensorPin);
    
  } else {
    // Offline
    tft.setTextColor(TFT_RED);
    tft.setCursor(x + 5, y + 28);
    tft.print("● OFFLINE");
    
    tft.setTextColor(TFT_DARKGREY);
    tft.setTextSize(2);
    tft.setCursor(x + 20, y + 70);
    tft.println("---");
  }
}

void updateStatusBar() {
  int statusY = tft.height() - 18;
  tft.fillRect(0, statusY, tft.width(), 20, COLOR_BG);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(10, statusY + 2);
  
  int connected = 0;
  if (loveSausageReceived) connected++;
  if (pussPussReceived) connected++;
  if (lamplighterReceived) connected++;
  
  tft.print("Devices: ");
  tft.print(connected);
  tft.print("/3  |  Uptime: ");
  tft.print(millis() / 1000);
  tft.print("s  |  ");
  
  // Show signal quality
  if (connected > 0) {
    tft.setTextColor(TFT_GREEN);
    tft.print("RX: OK");
  } else {
    tft.setTextColor(TFT_RED);
    tft.print("Waiting...");
  }
}

void updateDisplay() {
  // Redraw all sections
  int screenWidth = tft.width();
  int sectionWidth = (screenWidth - 40) / 3;
  int sectionX1 = 10;
  int sectionX2 = 20 + sectionWidth;
  int sectionX3 = 30 + (sectionWidth * 2);
  
  drawDeviceSection(sectionX1, 55, sectionWidth, 180, "Love Sausage", COLOR_LOVE_SAUSAGE, loveSausageReceived);
  drawDeviceSection(sectionX2, 55, sectionWidth, 180, "Puss-Puss", COLOR_PUSS_PUSS, pussPussReceived);
  drawDeviceSection(sectionX3, 55, sectionWidth, 180, "Lamplighter", COLOR_LAMPLIGHTER, lamplighterReceived);
  updateStatusBar();
}
