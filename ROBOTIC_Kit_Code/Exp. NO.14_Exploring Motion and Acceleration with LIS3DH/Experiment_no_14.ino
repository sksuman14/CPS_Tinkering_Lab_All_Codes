#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// Initialize LIS3DH using I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

void setup() {
  Serial.begin(9600);
  
  // Initialize sensor
  if (!lis.begin(0x18)) { // 0x18 is the default I2C address
    Serial.println("LIS3DH not found! Check connections.");
    while (1); // Halt execution
  }
  
  Serial.println("LIS3DH Found!");
  
  // Set range: ±2G, ±4G, ±8G, ±16G
  lis.setRange(LIS3DH_RANGE_2_G); // Default is ±2G

  // Set data rate (sampling speed)
  lis.setDataRate(LIS3DH_DATARATE_100_HZ); // 100 Hz
}

void loop() {
  lis.read(); // Read sensor data
  
  // Get X, Y, Z values in G-force
  Serial.print("X: "); Serial.print(lis.x / 16384.0); // Convert to G
  Serial.print(" Y: "); Serial.print(lis.y / 16384.0);
  Serial.print(" Z: "); Serial.print(lis.z / 16384.0);
  Serial.println(" G");
  
  delay(500); // Delay for readability
}
