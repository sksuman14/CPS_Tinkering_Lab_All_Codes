#include <Wire.h>
#include <SensirionI2cSts3x.h>

SensirionI2cSts3x sts30;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  Serial.println("STS30 Temperature Sensor");

  // Initialize STS30
  sts30.begin(Wire, 0x4A);

  Serial.println("STS30 initialized successfully!");
}

void loop() {

  float temperature = 0;

  // Take a single temperature measurement
  int16_t error = sts30.measureSingleShot(
    REPEATABILITY_MEDIUM,
    false,
    temperature
  );

  if (error == 0) {
    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" °C");
  } 
  else {
    Serial.print("STS30 reading failed. Error: ");
    Serial.println(error);
  }

  Serial.println("------------------------");

  delay(1000);
}

