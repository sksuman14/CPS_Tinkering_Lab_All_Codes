#include <Wire.h>
#include <Adafruit_VEML7700.h>
#include <EEPROM.h>

Adafruit_VEML7700 veml = Adafruit_VEML7700();


// =======================================================
// UART COMMAND SETTINGS
// =======================================================

#define CMD_BUF_SIZE 64

char cmd_buf[CMD_BUF_SIZE];
int cmd_len = 0;


// =======================================================
// READING INTERVAL
// Default = 1 second
// =======================================================

int interval_ms = 1000;

bool interval_updated = false;


// =======================================================
// EEPROM
// =======================================================

#define EEPROM_INTERVAL_ADDR 0


// =======================================================
// VEML7700 I2C ADDRESS
// =======================================================

#define VEML7700_ADDRESS 0x10


// =======================================================
// CHECK IF VEML7700 IS CONNECTED
// =======================================================

bool sensorConnected()
{
  Wire.beginTransmission(VEML7700_ADDRESS);

  if (Wire.endTransmission() == 0)
  {
    return true;
  }

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


    // Check if a number was received
    if (isdigit((unsigned char)*p))
    {
      int value = atoi(p);


      if (value > 0)
      {
        // Convert seconds to milliseconds
        interval_ms = value * 1000;

        interval_updated = true;


        // Save interval in EEPROM
        EEPROM.put(EEPROM_INTERVAL_ADDR, interval_ms);


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

  memset(cmd_buf, 0, sizeof(cmd_buf));
}


// =======================================================
// READ UART COMMANDS
//
// A command is processed when:
// \r or \n is received
// =======================================================

void ui_poll()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();


    // ---------------------------------------------------
    // End of command
    // ---------------------------------------------------

    if (c == '\r' || c == '\n')
    {
      if (cmd_len > 0)
      {
        cmd_buf[cmd_len] = '\0';

        process_commands(cmd_buf);

        cmd_len = 0;

        memset(cmd_buf, 0, sizeof(cmd_buf));
      }
    }


    // ---------------------------------------------------
    // Store received character
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


  Serial.println("VEML7700 Ambient Light Sensor");


  // -----------------------------------------------------
  // LOAD SAVED INTERVAL FROM EEPROM
  // -----------------------------------------------------

  EEPROM.get(
    EEPROM_INTERVAL_ADDR,
    interval_ms
  );


  // -----------------------------------------------------
  // SAFETY CHECK
  // -----------------------------------------------------

  if (interval_ms <= 0)
  {
    interval_ms = 1000;
  }


  Serial.print("Current interval: ");

  Serial.print(interval_ms / 1000);

  Serial.println(" seconds");


  // -----------------------------------------------------
  // INITIALIZE VEML7700
  // -----------------------------------------------------

  if (!veml.begin())
  {
    Serial.println("VEML7700 not found!");

    while (1)
    {
      delay(10);
    }
  }


  Serial.println(
    "VEML7700 detected successfully!"
  );


  Serial.println("VEML7700 ready.");
}


// =======================================================
// LOOP
// =======================================================

void loop()
{
  // -----------------------------------------------------
  // CHECK SENSOR CONNECTION
  // -----------------------------------------------------

  if (!sensorConnected())
  {
    Serial.println("Light: N/A");

    Serial.println("VEML7700 not connected!");
  }
  else
  {
    // ---------------------------------------------------
    // READ LIGHT
    // ---------------------------------------------------

    float lux = veml.readLux();


    // ---------------------------------------------------
    // PRINT LIGHT VALUE
    // ---------------------------------------------------

    Serial.print("Light: ");

    Serial.print(lux);

    Serial.println(" lux");
  }


  // -----------------------------------------------------
  // WAIT FOR CONFIGURED INTERVAL
  //
  // UART is checked every 100 ms.
  // -----------------------------------------------------

  int elapsed_ms = 0;

  interval_updated = false;


  while (elapsed_ms < interval_ms)
  {
    // Check for incoming commands
    ui_poll();


    // ---------------------------------------------------
    // If interval was changed,
    // immediately start next reading.
    // ---------------------------------------------------

    if (interval_updated)
    {
      interval_updated = false;

      break;
    }


    // ---------------------------------------------------
    // Check UART every 100 ms
    // ---------------------------------------------------

    delay(100);

    elapsed_ms += 100;
  }
}
