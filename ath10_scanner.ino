#include <Wire.h>

#define I2C_SDA 8
#define I2C_SCL 9

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("\nI2C Scanner");
  
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // Identify AHT10
      if (address == 0x38) {
        Serial.println(" (AHT10/AHT15/AHT20!)");
      } else if (address == 0x76 || address == 0x77) {
        Serial.println(" (BME280/BMP280!)");
      } else {
        Serial.println(" !");
      }
      nDevices++;
    }
  }
  
  if (nDevices == 0) {
    Serial.println("No I2C devices found - check wiring!");
  } else {
    Serial.println("Done scanning.");
  }
}

void loop() {}
