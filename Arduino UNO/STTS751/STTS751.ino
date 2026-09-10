#include <Wire.h>

#define STTS751_ADDRESS 0x4A

void setup() {
  Serial.begin(9600);
  Wire.begin();

  delay(100);

  Serial.println("STTS751 Temperature Sensor");
}

void loop() {
  // Tell STTS751 that we want temperature high byte
  Wire.beginTransmission(STTS751_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();

  // Read temperature high byte and low byte
  Wire.requestFrom(STTS751_ADDRESS, 2);

  if (Wire.available() >= 2) {

    int8_t tempHigh = Wire.read();
    byte tempLow = Wire.read();

    // STTS751 default resolution is 10-bit:
    // low byte bits 7 and 6 represent 0.5 and 0.25 degrees
    float temperature = tempHigh;

    if (tempLow & 0x80) {
      temperature += 0.5;
    }

    if (tempLow & 0x40) {
      temperature += 0.25;
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
  else {
    Serial.println("STTS751 not found!");
  }

  delay(1000);
}