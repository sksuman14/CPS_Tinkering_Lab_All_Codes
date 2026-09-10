/*
  Component: SparkFun LIS2DH12 Accelerometer (Normal Version)
  Target: Arduino Mega 2560
  Library: SparkFun LIS2DH12 Arduino Library
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <SparkFun_LIS2DH12.h>

SPARKFUN_LIS2DH12 myIC;

// Conversion factor: 1 mg = 0.00980665 m/s^2
const float MG_TO_MS2 = 0.00980665;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  Serial.println(F("SparkFun LIS2DH12 Test Starting..."));

  // Explicitly set the I2C address to 0x18
  if (myIC.begin(0x18) == false) {
    Serial.println(F("Could not find a valid LIS2DH12 sensor at 0x18, check wiring!"));
    while (1) { delay(10); }
  }
  
  Serial.println(F("LIS2DH12 ready."));
}

void loop() {
  // Read acceleration values in mg, then convert to m/s^2
  float x = myIC.getX() * MG_TO_MS2;
  float y = myIC.getY() * MG_TO_MS2;
  float z = myIC.getZ() * MG_TO_MS2;
  
  Serial.print(F("LIS2DH: X="));
  Serial.print(x, 2);
  Serial.print(F(", Y="));
  Serial.print(y, 2);
  Serial.print(F(", Z="));
  Serial.println(z, 2);

  delay(1000);
}