/*
  Component 9: STS30 Temperature Sensor (Celsius & Fahrenheit)
  Target: Arduino Mega 2560
  Library: None (Direct I2C communication)
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>

#define STS30_ADDR 0x4A 

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10); 
  
  Wire.begin();
  
  Serial.println("STS30 Temperature Sensor (C & F) Test Started");
}

void loop() {
  Wire.beginTransmission(STS30_ADDR);
  Wire.write(0x24); 
  Wire.write(0x00);
  Wire.endTransmission();

  delay(20); 

  Wire.requestFrom(STS30_ADDR, 3);
  
  if (Wire.available() == 3) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t crc = Wire.read(); // Read checksum byte but do not process it

    uint16_t rawTemp = (msb << 8) | lsb;

    // Convert raw data to Celsius
    float tempCelsius = -45.0 + (175.0 * ((float)rawTemp / 65535.0));
    
    // Convert Celsius to Fahrenheit
    float tempFahrenheit = (tempCelsius * 1.8) + 32.0;

    // Print both values side-by-side
    Serial.print("Temperature: ");
    Serial.print(tempCelsius);
    Serial.print(" C  |  ");
    Serial.print(tempFahrenheit);
    Serial.println(" F");
  } else {
    Serial.println("Error: Sensor not responding.");
  }

  delay(1000);
}