/*
  Component 6: STTS751 Temperature Sensor
  Target: Arduino Mega 2560
  Library: STM32duino STTS751
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <STTS751Sensor.h>

// Create the sensor object and pass the I2C Wire interface to it
STTS751Sensor TempSensor(&Wire);

void setup() {
  Serial.begin(9600);
  
  // Initialize the I2C bus
  Wire.begin();
  
  // Initialize and enable the STM32duino sensor
  TempSensor.begin();
  TempSensor.Enable();
  
  Serial.println("STM32duino STTS751 initialized.");
}

void loop() {
  float temperature = 0;
  
  // The library passes the value into the variable by reference
  TempSensor.GetTemperature(&temperature);
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");
  
  delay(1000); // Read every 1 second
}