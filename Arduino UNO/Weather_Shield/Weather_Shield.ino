#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_VEML7700.h>

// Sensor objects
Adafruit_BME680 bme;
Adafruit_VEML7700 veml;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("BME680 + VEML7700 Sensor");

  // Initialize BME680
  if (!bme.begin(0x76)) {
    Serial.println("BME680 not found!");
    while (1);
  }

  // BME680 configuration
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  Serial.println("BME680 detected successfully!");

  // Initialize VEML7700
  if (!veml.begin()) {
    Serial.println("VEML7700 not found!");
    while (1);
  }

  Serial.println("VEML7700 detected successfully!");
  Serial.println("Sensors ready.");
}

void loop() {

  // Read BME680
  if (bme.performReading()) {

    float temperature = bme.temperature;
    float humidity = bme.humidity;
    float pressure = bme.pressure / 100.0;

    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity, 2);
    Serial.println(" %");

    Serial.print("Pressure: ");
    Serial.print(pressure, 2);
    Serial.println(" hPa");

  } else {
    Serial.println("BME680 reading failed!");
  }

  // Read VEML7700
  float lux = veml.readLux();

  Serial.print("Light: ");
  Serial.print(lux, 2);
  Serial.println(" lux");

  Serial.println("------------------------");

  delay(1000);
}
