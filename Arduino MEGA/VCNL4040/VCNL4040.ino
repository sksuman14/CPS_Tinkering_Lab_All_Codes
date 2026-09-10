/*
  Component 7: VCNL4040 Ambient Light (Lux) Only
  Target: Arduino Mega 2560
  Library: Adafruit VCNL4040
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <Adafruit_VCNL4040.h>

Adafruit_VCNL4040 vcnl4040 = Adafruit_VCNL4040();

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10); 

  Serial.println("VCNL4040 Lux Sensor Test");

  if (!vcnl4040.begin()) {
    Serial.println("Could not find a valid VCNL4040 sensor, check wiring!");
    while (1) delay(10); 
  }
}

void loop() {
  // Read only the Ambient Light value (in Lux)
  uint16_t ambientLight = vcnl4040.getLux();

  // Print the lux value to the Serial Monitor
  Serial.print("Ambient Light: ");
  Serial.print(ambientLight);
  Serial.println(" Lux");

  delay(500);
}