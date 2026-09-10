#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

void setup() {
  Serial.begin(9600);

  // Start LIS3DH using I2C
  if (!lis.begin(0x19)) {
    Serial.println("Could not find LIS3DH!");
    while (1);
  }

  Serial.println("LIS3DH found!");

  // Set measurement range
  lis.setRange(LIS3DH_RANGE_2_G);

  // Set data rate
  lis.setDataRate(LIS3DH_DATARATE_100_HZ);
}

void loop() {
  sensors_event_t event;

  lis.getEvent(&event);

  Serial.print("X: ");
  Serial.print(event.acceleration.x);
  Serial.print(" m/s^2");

  Serial.print("   Y: ");
  Serial.print(event.acceleration.y);
  Serial.print(" m/s^2");

  Serial.print("   Z: ");
  Serial.print(event.acceleration.z);
  Serial.println(" m/s^2");

  delay(1000);
}






