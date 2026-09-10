#include <Wire.h>
#include <SensirionI2cSts3x.h>
#include <EEPROM.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


// =======================================================
// STS30 SENSOR
// =======================================================

SensirionI2cSts3x sts30;


// STS30 I2C address
#define STS30_ADDRESS 0x4A


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

int interval_ms = 1000;

bool interval_updated = false;


// =======================================================
// EEPROM
// =======================================================

#define EEPROM_INTERVAL_ADDR 0


// =======================================================
// SENSOR STATE
// =======================================================

bool sensor_connected = false;


// =======================================================
// CHECK STS30 I2C CONNECTION
// =======================================================

bool checkSTS30()
{
  Wire.beginTransmission(STS30_ADDRESS);

  byte error = Wire.endTransmission();

  if (error == 0)
  {
    return true;
  }

  return false;
}


// =======================================================
// INITIALIZE STS30
// =======================================================

bool initializeSTS30()
{
  Serial.println("Checking STS30...");

  // Initialize STS30
  sts30.begin(Wire, STS30_ADDRESS);

  // Check if device responds on I2C
  if (checkSTS30())
  {
    Serial.println("STS30 detected successfully!");

    sensor_connected = true;

    return true;
  }

  Serial.println("STS30 not connected!");

  sensor_connected = false;

  return false;
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
  // 1. RESTART DEVICE
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
  // 2. SET_INTERVAL:<seconds>
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


    // Check if a number exists
    if (isdigit((unsigned char)*p))
    {
      int value = atoi(p);


      if (value > 0)
      {
        // Convert seconds to milliseconds
        interval_ms = value * 1000;

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
// READ UART COMMANDS
//
// Command is processed when:
// \r or \n is received
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
// SETUP
// =======================================================

void setup()
{
  // -----------------------------------------------------
  // UART
  // -----------------------------------------------------

  Serial.begin(115200);


  // -----------------------------------------------------
  // I2C
  // -----------------------------------------------------

  Wire.begin();


  Serial.println();
  Serial.println("==============================");
  Serial.println("STS30 Temperature Sensor");
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

  if (interval_ms <= 0 ||
      interval_ms > 86400000)
  {
    interval_ms = 1000;
  }


  Serial.print("Current interval: ");

  Serial.print(interval_ms / 1000);

  Serial.println(" seconds");


  // -----------------------------------------------------
  // INITIALIZE STS30
  // -----------------------------------------------------

  initializeSTS30();


  if (sensor_connected)
  {
    Serial.println("STS30 ready.");
  }
  else
  {
    Serial.println(
      "Waiting for STS30 connection..."
    );
  }
}


// =======================================================
// LOOP
// =======================================================

void loop()
{
  // -----------------------------------------------------
  // CHECK SENSOR CONNECTION
  // -----------------------------------------------------

  bool currently_connected = checkSTS30();


  // =====================================================
  // SENSOR DISCONNECTED
  // =====================================================

  if (!currently_connected)
  {
    // Sensor was previously connected
    if (sensor_connected)
    {
      Serial.println();
      Serial.println("STS30 disconnected!");
    }


    sensor_connected = false;


    // Do NOT print old temperature.
    Serial.println("Temperature: N/A");


    // ---------------------------------------------------
    // Wait while checking for reconnection
    // ---------------------------------------------------

    int elapsed_ms = 0;

    interval_updated = false;


    while (elapsed_ms < interval_ms)
    {
      // Check UART commands
      ui_poll();


      // If interval changed,
      // start another sensor cycle
      if (interval_updated)
      {
        interval_updated = false;

        break;
      }


      // Check every 100 ms
      delay(100);

      elapsed_ms += 100;
    }


    return;
  }


  // =====================================================
  // SENSOR HAS BEEN RECONNECTED
  // =====================================================

  if (!sensor_connected)
  {
    Serial.println();
    Serial.println("STS30 detected!");
    Serial.println("Reinitializing sensor...");


    // Reinitialize STS30
    if (initializeSTS30())
    {
      Serial.println("Sensor recovered.");
    }
    else
    {
      Serial.println("Sensor initialization failed.");

      return;
    }
  }


  // =====================================================
  // READ STS30
  // =====================================================

  float temperature = 0.0;


  int16_t error = sts30.measureSingleShot(
    REPEATABILITY_MEDIUM,
    false,
    temperature
  );


  // =====================================================
  // CHECK READING
  // =====================================================

  if (error != 0)
  {
    Serial.println("Temperature: N/A");

    Serial.print("STS30 read error: ");
    Serial.println(error);

    sensor_connected = false;
  }
  else
  {
    // ---------------------------------------------------
    // TEMPERATURE
    // ---------------------------------------------------

    Serial.print("Temp: ");

    Serial.print(
      temperature,
      2
    );

    Serial.println(" °C");

  }


  // =====================================================
  // WAIT FOR CONFIGURED INTERVAL
  // =====================================================

  int elapsed_ms = 0;

  interval_updated = false;


  while (elapsed_ms < interval_ms)
  {
    // ---------------------------------------------------
    // Check UART commands
    // ---------------------------------------------------

    ui_poll();


    // ---------------------------------------------------
    // If interval changed,
    // immediately start next reading
    // ---------------------------------------------------

    if (interval_updated)
    {
      interval_updated = false;

      break;
    }


    // ---------------------------------------------------
    // Wait 100 ms
    // ---------------------------------------------------

    delay(100);

    elapsed_ms += 100;
  }
}

