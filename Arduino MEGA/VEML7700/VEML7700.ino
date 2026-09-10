/*
  Component 8: VEML7700 Ambient Light Sensor
  Target: Arduino Mega 2560
  Library: Adafruit VEML7700
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <Adafruit_VEML7700.h>

// Create the sensor object
Adafruit_VEML7700 veml = Adafruit_VEML7700();

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10); // Wait for Serial Monitor

  Serial.println("VEML7700 Ambient Light Sensor Test");

  // Initialize the sensor (Default I2C address is 0x10)
  if (!veml.begin()) {
    Serial.println("Could not find a valid VEML7700 sensor, check wiring!");
    while (1) delay(10); // Halt if sensor not found
  }
  
  Serial.println("VEML7700 found!");
  
  // The Adafruit library sets excellent default values for Gain and Integration Time.
  // We will use the defaults, which work great for standard indoor lighting.
}

void loop() {
  // Read the calculated Lux value
  // This function returns a floating-point number (e.g., 245.50)
  float lux = veml.readLux();
  
  // Print the lux value to the Serial Monitor
  Serial.print("Ambient Light: ");
  Serial.print(lux);
  Serial.println(" Lux");

  // Wait 500 milliseconds before reading again
  delay(500);
}