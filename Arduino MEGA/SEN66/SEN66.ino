/*
  Component 10: Sensirion SEN66 Environmental Sensor
  Target: Arduino Mega 2560
  Library: Sensirion I2C SEN66
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21) + EXTERNAL 3.3V POWER
*/

#include <Wire.h>
#include <SensirionI2cSen66.h> 

SensirionI2cSen66 sen66;

void setup() {
  Serial.begin(9600); 
  while (!Serial) { delay(100); }

  Wire.begin();
  
  // FIX: Pass both the Wire interface and the I2C address (0x6B)
  sen66.begin(Wire, 0x6B);

  Serial.println("Initializing SEN66...");

  // Reset the sensor to ensure a clean state
  uint16_t error = sen66.deviceReset();
  if (error) {
    Serial.println("Error resetting SEN66. Check wiring and power!");
    while(1) delay(10);
  }
  delay(1200); // Wait for the sensor to boot up after reset

  // Start continuous measurement
  error = sen66.startContinuousMeasurement();
  if (error) {
    Serial.println("Error starting measurement.");
    while(1) delay(10);
  }
  
  Serial.println("SEN66 running.");
}

void loop() {
  uint16_t error;
  
  float massConcentrationPm1p0;
  float massConcentrationPm2p5;
  float massConcentrationPm4p0;
  float massConcentrationPm10p0;
  float ambientHumidity;
  float ambientTemperature;
  float vocIndex;
  float noxIndex;
  uint16_t co2;

  // Read all data at once
  error = sen66.readMeasuredValues(
      massConcentrationPm1p0, massConcentrationPm2p5,
      massConcentrationPm4p0, massConcentrationPm10p0,
      ambientHumidity, ambientTemperature, 
      vocIndex, noxIndex, co2);

  if (error) {
    Serial.println("Error reading data from SEN66");
  } else {
    Serial.println("----- SEN66 Readings -----");
    
    Serial.print("PM1.0: "); Serial.print(massConcentrationPm1p0); Serial.println(" ug/m3");
    Serial.print("PM2.5: "); Serial.print(massConcentrationPm2p5); Serial.println(" ug/m3");
    Serial.print("PM4.0: "); Serial.print(massConcentrationPm4p0); Serial.println(" ug/m3");
    Serial.print("PM10 : "); Serial.print(massConcentrationPm10p0); Serial.println(" ug/m3");

    Serial.print("Temperature: "); Serial.print(ambientTemperature); Serial.println(" C");
    Serial.print("Humidity: "); Serial.print(ambientHumidity); Serial.println(" %RH");

    Serial.print("VOC Index: "); Serial.print(vocIndex); Serial.println();
    Serial.print("NOx Index: "); Serial.print(noxIndex); Serial.println();
    Serial.print("CO2: "); Serial.print(co2); Serial.println(" ppm");
    
    Serial.println("--------------------------\n");
  }

  delay(1000);
}