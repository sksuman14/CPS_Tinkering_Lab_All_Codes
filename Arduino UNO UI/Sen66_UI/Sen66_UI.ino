#include <Wire.h>
#include <SensirionI2cSen66.h>
#include <EEPROM.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


// =======================================================
// SEN66 SENSOR
// =======================================================

SensirionI2cSen66 sen66;

#define SEN66_ADDRESS 0x6B


// =======================================================
// UART COMMAND SETTINGS
// =======================================================

#define CMD_BUF_SIZE 64

char cmd_buf[CMD_BUF_SIZE];
int cmd_len = 0;


// =======================================================
// REPORTING INTERVAL
// Default = 1 second
// =======================================================

unsigned long interval_ms = 1000;

bool interval_updated = false;


// =======================================================
// EEPROM
// =======================================================

#define EEPROM_INTERVAL_ADDR 0


// =======================================================
// SENSOR STATE
// =======================================================

bool sensor_connected = false;
bool measurement_running = false;


// =======================================================
// CHECK SEN66 CONNECTION
// =======================================================

bool checkSEN66()
{
  Wire.beginTransmission(SEN66_ADDRESS);

  byte error = Wire.endTransmission();

  if (error == 0)
  {
    return true;
  }

  return false;
}


// =======================================================
// INITIALIZE SEN66
// =======================================================

bool initializeSEN66()
{
  Serial.println("Checking SEN66...");

  // Check I2C connection
  if (!checkSEN66())
  {
    Serial.println("SEN66 not connected!");

    sensor_connected = false;
    measurement_running = false;

    return false;
  }


  // Initialize SEN66
  sen66.begin(Wire, SEN66_ADDRESS);


  // Start continuous measurement
  int16_t error = sen66.startContinuousMeasurement();


  if (error != 0)
  {
    Serial.print("Failed to start SEN66. Error: ");
    Serial.println(error);

    sensor_connected = false;
    measurement_running = false;

    return false;
  }


  sensor_connected = true;
  measurement_running = true;


  Serial.println("SEN66 detected successfully!");
  Serial.println("Continuous measurement started.");

  return true;
}


// =======================================================
// PROCESS UART COMMANDS
//
// Supported commands:
//
// restartDevice
// SET_INTERVAL:<seconds>
//
// Example:
//
// SET_INTERVAL:5
// =======================================================

void process_commands(char *buf)
{
  // -----------------------------------------------------
  // RESTART DEVICE
  // -----------------------------------------------------

  if (strstr(buf, "restartDevice"))
  {
    Serial.println("Rebooting...");

    delay(50);


    // Software reset
    void (*resetFunc)(void) = 0;

    resetFunc();

    return;
  }


  // -----------------------------------------------------
  // SET_INTERVAL:<seconds>
  // -----------------------------------------------------

  char *p = strstr(buf, "SET_INTERVAL:");


  if (p)
  {
    p += strlen("SET_INTERVAL:");


    // Skip non-digit characters
    while (*p != '\0' &&
           !isdigit((unsigned char)*p))
    {
      p++;
    }


    if (isdigit((unsigned char)*p))
    {
      unsigned long value = strtoul(
        p,
        NULL,
        10
      );


      if (value > 0)
      {
        // Seconds -> milliseconds
        interval_ms = value * 1000UL;

        interval_updated = true;


        // Save interval to EEPROM
        EEPROM.put(
          EEPROM_INTERVAL_ADDR,
          interval_ms
        );


        Serial.print("Interval updated to ");

        Serial.print(value);

        Serial.println(" seconds");
      }
      else
      {
        Serial.println(
          "Invalid interval value. Must be greater than 0."
        );
      }
    }
    else
    {
      Serial.println("Invalid interval value.");
    }
  }


  // -----------------------------------------------------
  // Clear command buffer
  // -----------------------------------------------------

  cmd_len = 0;

  memset(
    cmd_buf,
    0,
    sizeof(cmd_buf)
  );
}


// =======================================================
// UART POLLING
// =======================================================

void ui_poll()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();


    // ---------------------------------------------------
    // END OF COMMAND
    // ---------------------------------------------------

    if (c == '\r' || c == '\n')
    {
      if (cmd_len > 0)
      {
        cmd_buf[cmd_len] = '\0';

        process_commands(cmd_buf);

        cmd_len = 0;

        memset(
          cmd_buf,
          0,
          sizeof(cmd_buf)
        );
      }
    }


    // ---------------------------------------------------
    // STORE CHARACTER
    // ---------------------------------------------------

    else if (cmd_len < CMD_BUF_SIZE - 1)
    {
      cmd_buf[cmd_len++] = c;
    }
  }
}


// =======================================================
// PRINT N/A VALUES
// =======================================================

void printNA()
{
  Serial.println("PM1.0: N/A");
  Serial.println("PM2.5: N/A");
  Serial.println("PM4.0: N/A");
  Serial.println("PM10: N/A");
  Serial.println("Humidity: N/A");
  Serial.println("Temperature: N/A");
  Serial.println("VOC Index: N/A");
  Serial.println("NOx Index: N/A");
  Serial.println("CO2: N/A");

  Serial.println("------------------------");
}


// =======================================================
// WAIT USING CONFIGURED INTERVAL
//
// UART is checked every 100 ms.
// =======================================================

void waitForInterval()
{
  unsigned long elapsed_ms = 0;

  interval_updated = false;


  while (elapsed_ms < interval_ms)
  {
    // Check incoming commands
    ui_poll();


    // Interval changed
    if (interval_updated)
    {
      interval_updated = false;

      break;
    }


    // Check sensor connection
    if (!checkSEN66())
    {
      sensor_connected = false;
      measurement_running = false;

      Serial.println();
      Serial.println("SEN66 disconnected!");

      break;
    }


    delay(100);

    elapsed_ms += 100;
  }
}


// =======================================================
// SETUP
// =======================================================

void setup()
{
  // -----------------------------------------------------
  // UART
  // -----------------------------------------------------

  Serial.begin(9600);


  // -----------------------------------------------------
  // I2C
  // -----------------------------------------------------

  Wire.begin();


  Serial.println();
  Serial.println("==============================");
  Serial.println("SEN66 Air Quality Sensor");
  Serial.println("==============================");


  // -----------------------------------------------------
  // LOAD INTERVAL FROM EEPROM
  // -----------------------------------------------------

  EEPROM.get(
    EEPROM_INTERVAL_ADDR,
    interval_ms
  );


  // -----------------------------------------------------
  // SAFETY CHECK
  // -----------------------------------------------------

  if (interval_ms == 0 ||
      interval_ms > 86400000UL)
  {
    interval_ms = 1000;
  }


  Serial.print("Current interval: ");

  Serial.print(
    interval_ms / 1000UL
  );

  Serial.println(" seconds");


  // -----------------------------------------------------
  // INITIALIZE SEN66
  // -----------------------------------------------------

  if (initializeSEN66())
  {
    Serial.println("SEN66 ready.");

    // Allow first measurement to become available
    delay(1000);
  }
  else
  {
    Serial.println(
      "Waiting for SEN66 connection..."
    );
  }
}


// =======================================================
// LOOP
// =======================================================

void loop()
{
  // =====================================================
  // CHECK SENSOR CONNECTION
  // =====================================================

  if (!checkSEN66())
  {
    // ---------------------------------------------------
    // Sensor disconnected
    // ---------------------------------------------------

    if (sensor_connected)
    {
      Serial.println();
      Serial.println("SEN66 disconnected!");
    }


    sensor_connected = false;
    measurement_running = false;


    // Don't print old values
    printNA();


    // Wait and check for reconnection
    waitForInterval();

    return;
  }


  // =====================================================
  // SENSOR RECONNECTED
  // =====================================================

  if (!sensor_connected)
  {
    Serial.println();
    Serial.println("SEN66 detected!");
    Serial.println("Reinitializing sensor...");


    if (initializeSEN66())
    {
      Serial.println("SEN66 recovered.");

      // Allow measurement to start
      delay(1000);
    }
    else
    {
      Serial.println(
        "SEN66 initialization failed."
      );

      return;
    }
  }


  // =====================================================
  // CHECK DATA READY
  // =====================================================

  uint8_t padding = 0;
  bool dataReady = false;


  int16_t error = sen66.getDataReady(
    padding,
    dataReady
  );


  if (error != 0)
  {
    Serial.println(
      "SEN66 data-ready check failed."
    );

    printNA();

    waitForInterval();

    return;
  }


  // -----------------------------------------------------
  // Data not ready
  // -----------------------------------------------------

  if (!dataReady)
  {
    waitForInterval();

    return;
  }


  // =====================================================
  // SENSOR VARIABLES
  // =====================================================

  float pm1_0 = 0.0;
  float pm2_5 = 0.0;
  float pm4_0 = 0.0;
  float pm10 = 0.0;

  float humidity = 0.0;
  float temperature = 0.0;

  float vocIndex = 0.0;
  float noxIndex = 0.0;

  uint16_t co2 = 0;


  // =====================================================
  // READ SEN66
  // =====================================================

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


  // =====================================================
  // CHECK READ ERROR
  // =====================================================

  if (error != 0)
  {
    Serial.println("Failed to read SEN66!");

    Serial.print("Error: ");
    Serial.println(error);

    printNA();

    sensor_connected = false;

    return;
  }


  // =====================================================
  // PRINT VALUES
  // =====================================================

  Serial.print("PM1.0: ");
  Serial.print(pm1_0, 2);
  Serial.print(" ug/m3, ");


  Serial.print("PM2.5: ");
  Serial.print(pm2_5, 2);
  Serial.print(" ug/m3, ");


  Serial.print("PM4: ");
  Serial.print(pm4_0, 2);
  Serial.print(" ug/m3, ");


  Serial.print("PM10: ");
  Serial.print(pm10, 2);
  Serial.print(" ug/m3, ");


  Serial.print("Relative Humidity: ");
  Serial.print(humidity, 2);
  Serial.print(" %RH, ");


  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.print(" °C, ");


  Serial.print("VOC: ");
  Serial.print(vocIndex, 2);
  Serial.print(", ");


  Serial.print("NOx: ");
  Serial.print(noxIndex, 2);
  Serial.print(", ");


  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.println(" ppm, ");




  // =====================================================
  // WAIT FOR NEXT READING
  // =====================================================

  waitForInterval();
}

