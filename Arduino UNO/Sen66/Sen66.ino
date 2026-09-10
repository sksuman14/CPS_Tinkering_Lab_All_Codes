#include <Wire.h>
#include <SensirionI2cSen66.h>

SensirionI2cSen66 sen66;

#define SEN66_ADDRESS 0x6B

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  Serial.println("SEN66 Air Quality Sensor");

  // Initialize SEN66
  sen66.begin(Wire, SEN66_ADDRESS);

  // Start continuous measurement
  int16_t error = sen66.startContinuousMeasurement();

  if (error != 0)
  {
    Serial.print("Failed to start SEN66. Error: ");
    Serial.println(error);

    while (1)
    {
      delay(10);
    }
  }

  Serial.println("SEN66 detected successfully!");
  Serial.println("Continuous measurement started.");

  // Give the sensor some time to start measuring
  delay(1000);
}

void loop()
{
  // ---------------------------------------------------
  // Sensor variables
  // ---------------------------------------------------

  float pm1_0 = 0.0;
  float pm2_5 = 0.0;
  float pm4_0 = 0.0;
  float pm10 = 0.0;

  float humidity = 0.0;
  float temperature = 0.0;

  float vocIndex = 0.0;
  float noxIndex = 0.0;

  uint16_t co2 = 0;


  // ---------------------------------------------------
  // Check if data is ready
  // ---------------------------------------------------

  uint8_t padding = 0;
  bool dataReady = false;

  int16_t error = sen66.getDataReady(
    padding,
    dataReady
  );

  if (error != 0)
  {
    Serial.print("Data ready check failed. Error: ");
    Serial.println(error);

    delay(1000);
    return;
  }


  // ---------------------------------------------------
  // Data not ready
  // ---------------------------------------------------

  if (!dataReady)
  {
    delay(100);
    return;
  }


  // ---------------------------------------------------
  // Read measured values
  // ---------------------------------------------------

  error = sen66.readMeasuredValues(
    pm1_0,
    pm2_5,
    pm4_0,
    pm10,
    humidity,
    temperature,
    vocIndex,
    noxIndex,
    co2
  );


  if (error != 0)
  {
    Serial.print("Failed to read SEN66. Error: ");
    Serial.println(error);

    delay(1000);
    return;
  }


  // ---------------------------------------------------
  // Print PM1.0
  // ---------------------------------------------------

  Serial.print("PM1.0: ");
  Serial.print(pm1_0, 2);
  Serial.println(" ug/m3");


  // ---------------------------------------------------
  // Print PM2.5
  // ---------------------------------------------------

  Serial.print("PM2.5: ");
  Serial.print(pm2_5, 2);
  Serial.println(" ug/m3");


  // ---------------------------------------------------
  // Print PM4.0
  // ---------------------------------------------------

  Serial.print("PM4.0: ");
  Serial.print(pm4_0, 2);
  Serial.println(" ug/m3");


  // ---------------------------------------------------
  // Print PM10
  // ---------------------------------------------------

  Serial.print("PM10: ");
  Serial.print(pm10, 2);
  Serial.println(" ug/m3");


  // ---------------------------------------------------
  // Print Humidity
  // ---------------------------------------------------

  Serial.print("Humidity: ");
  Serial.print(humidity, 2);
  Serial.println(" %RH");


  // ---------------------------------------------------
  // Print Temperature
  // ---------------------------------------------------

  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");


  // ---------------------------------------------------
  // Print VOC Index
  // ---------------------------------------------------

  Serial.print("VOC Index: ");
  Serial.println(vocIndex, 2);


  // ---------------------------------------------------
  // Print NOx Index
  // ---------------------------------------------------

  Serial.print("NOx Index: ");
  Serial.println(noxIndex, 2);


  // ---------------------------------------------------
  // Print CO2
  // ---------------------------------------------------

  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.println(" ppm");


  Serial.println("------------------------");

  delay(1000);
}

