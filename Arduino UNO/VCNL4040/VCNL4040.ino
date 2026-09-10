#include <Wire.h>
#include <Adafruit_VCNL4040.h>

Adafruit_VCNL4040 vcnl4040;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  Serial.println("VCNL4040 Sensor");

  if (!vcnl4040.begin()) {
    Serial.println("VCNL4040 not found!");
    while (1);
  }

  Serial.println("VCNL4040 found!");
}

void loop() {

  uint16_t proximity = vcnl4040.getProximity();
  uint16_t ambient = vcnl4040.getAmbientLight();
  uint16_t white = vcnl4040.getWhiteLight();
  uint16_t lux = vcnl4040.getLux();

  Serial.print("Proximity: ");
  Serial.println(proximity);

  Serial.print("Ambient Light: ");
  Serial.println(ambient);

  Serial.print("White Light: ");
  Serial.println(white);

  Serial.print("Lux: ");
  Serial.println(lux);

  Serial.println("-------------------");

  delay(500);
}