/*
  SHT40 Temperature & Humidity Sensor
  ------------------------------------
  Reads temperature (°C) and relative humidity (%) from the
  Sensirion SHT40 sensor over I2C.

  Library required (install via Arduino Library Manager):
    "Adafruit SHT4x Library" by Adafruit
    (also install its dependency: "Adafruit Unified Sensor")

  Wiring (I2C):
    SHT40 VCC -> 3.3V
    SHT40 GND -> GND
    SHT40 SDA -> A4 (Uno) / SDA pin (other boards)
    SHT40 SCL -> A5 (Uno) / SCL pin (other boards)

  Default I2C address: 0x44
*/

#include <Wire.h>
#include "Adafruit_SHT4x.h"

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10);

  Serial.println("SHT40 Temperature & Humidity Sensor");

  if (!sht4.begin()) {
    Serial.println("Couldn't find SHT40 sensor! Check wiring.");
    while (1) delay(10);
  }

  Serial.println("SHT40 found!");

  // Precision setting: higher precision = more accurate but slower
  sht4.setPrecision(SHT4X_HIGH_PRECISION);

  // Heater setting: keep off for normal readings
  sht4.setHeater(SHT4X_NO_HEATER);
}

void loop() {
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp); // populates temp and humidity objects

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  Serial.println("-----------------------------");

  delay(1000); // read once per second
}
