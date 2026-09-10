/*
  Component 5: LIS3DH 3-Axis Accelerometer
  Target: Arduino Mega 2560
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>

// Create the LIS3DH sensor object using I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

void setup() {
  Serial.begin(9600);
  
  // Wait for serial monitor to open (helpful for some boards)
  while (!Serial) delay(10); 

  Serial.println("LIS3DH Accelerometer Test");

  // Initialize the sensor. 
  // NOTE: Default I2C address is usually 0x18. If it fails, try 0x19.
  if (!lis.begin(0x19)) { 
    Serial.println("Could not find a valid LIS3DH sensor, check wiring!");
    while (1) delay(10); // Halt the code here if sensor is not found
  }
  
  Serial.println("LIS3DH found!");
  
  // Set the measuring range (2G, 4G, 8G, or 16G). 2G is best for basic tilt.
  lis.setRange(LIS3DH_RANGE_2_G); 
}

void loop() {
  // Create an 'event' object to hold the sensor data
  sensors_event_t event;
  
  // Ask the sensor to fill the event object with the latest readings
  lis.getEvent(&event);

  // Print the X, Y, and Z acceleration values (measured in m/s^2)
  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  \t");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  \t");
  Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  \t");
  Serial.println("m/s^2 ");

  // Read data every 500 milliseconds
  delay(500);
}