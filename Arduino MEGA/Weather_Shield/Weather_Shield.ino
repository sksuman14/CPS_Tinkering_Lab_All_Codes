/*
  Component 16: Weather Shield (BME680 + VEML7700 - Stripped UI)
  Target: Arduino Mega 2560
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_VEML7700.h>

// ---- Sensor objects ----
Adafruit_BME680   bme;
Adafruit_VEML7700 veml;

// ---- Sensor readings ----
double t = 0, h = 0, p = 0, l = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("BME680 + VEML7700 starting...");

  // Init BME680 at address 0x76
  if (!bme.begin(0x76)) {
    Serial.println("BME680 not found! Check wiring.");
    while (1);
  }
  
  // Configure BME680 parameters
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320 degrees C for 150ms

  // Init VEML7700 at default address 0x10
  if (!veml.begin()) {
    Serial.println("VEML7700 not found! Check wiring.");
    while (1);
  }

  Serial.println("BME680 + VEML7700 ready.");
}

void loop() {
  // ---- BME680 read ----
  if (bme.performReading()) {
    t = bme.temperature;
    h = bme.humidity;
    p = bme.pressure / 100.0; // Pa to hPa
  } else {
    Serial.println("Error: BME680 fetch failed");
    delay(1000);
    return;
  }

  // ---- VEML7700 read ----
  l = veml.readLux();

  // ---- Print data ----
  Serial.print("T:");  Serial.print(t, 2);
  Serial.print(",H:"); Serial.print(h, 2);
  Serial.print(",P:"); Serial.print(p, 2);
  Serial.print(",L:"); Serial.println(l, 2);

  // Wait 1 second before reading again
  delay(1000); 
}