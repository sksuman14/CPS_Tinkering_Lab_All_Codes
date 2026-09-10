/*
  Component 13: AHT20 Temperature & Humidity Sensor
  Target: Arduino Mega 2560
  Library: Adafruit AHTX0
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <Adafruit_AHTX0.h>

// Create the sensor object
Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10); // Wait for Serial Monitor

  Serial.println("AHT20 Temperature & Humidity Test");

  // Initialize the sensor (Default I2C address is 0x38)
  if (!aht.begin()) {
    Serial.println("Could not find a valid AHT20 sensor, check wiring!");
    while (1) delay(10); // Halt if sensor not found
  }
  
  Serial.println("AHT20 found!");
}

void loop() {
  // Create sensor event objects to hold the data
  sensors_event_t humidity, temp;
  
  // Populate the objects with fresh data from the sensor
  aht.getEvent(&humidity, &temp);

  // Print Temperature
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.print(" degrees C  |  ");

  // Print Humidity
  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" % rH");

  // Wait 1 second before reading again
  delay(1000);
}