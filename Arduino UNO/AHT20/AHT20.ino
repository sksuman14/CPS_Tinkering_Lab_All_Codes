#include <Wire.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  Serial.println("AHT20 Temperature and Humidity Sensor");

  // Initialize AHT20
  if (!aht.begin())
  {
    Serial.println("AHT20 not found!");

    while (1)
    {
      delay(10);
    }
  }

  Serial.println("AHT20 detected successfully!");
}

void loop()
{
  sensors_event_t humidity;
  sensors_event_t temperature;

  // Get sensor readings
  aht.getEvent(&humidity, &temperature);

  // Temperature
  Serial.print("Temperature: ");
  Serial.print(temperature.temperature);
  Serial.println(" °C");

  // Humidity
  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %RH");

  Serial.println("------------------------");

  delay(1000);
}