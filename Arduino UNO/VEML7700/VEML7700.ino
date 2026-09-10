#include <Wire.h>
#include <Adafruit_VEML7700.h>

Adafruit_VEML7700 veml = Adafruit_VEML7700();

void setup() {
  Serial.begin(9600);
  Wire.begin();

  Serial.println("VEML7700 Ambient Light Sensor");

  if (!veml.begin()) {
    Serial.println("VEML7700 not found!");
    while (1) {
      delay(10);
    }
  }

  Serial.println("VEML7700 detected successfully!");
}

void loop() {
  float lux = veml.readLux();

  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lux");

  delay(1000);
}