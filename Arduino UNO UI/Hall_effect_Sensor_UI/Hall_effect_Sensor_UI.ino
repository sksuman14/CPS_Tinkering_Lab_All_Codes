#include <EEPROM.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


// =======================================================
// HALL EFFECT SENSOR
// =======================================================

#define HALL_PIN 2


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

bool sensor_connected = true;


// =======================================================
// SENSOR INITIALIZATION
// =======================================================

void initializeHallSensor()
{
  pinMode(HALL_PIN, INPUT);

  sensor_connected = true;

  Serial.println("Hall Effect Sensor initialized!");
}


// =======================================================
// PROCESS UART COMMANDS
//
// Supported:
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


        // Save interval
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
// WAIT FOR CONFIGURED INTERVAL
// =======================================================

void waitForInterval()
{
  unsigned long elapsed_ms = 0;

  interval_updated = false;


  while (elapsed_ms < interval_ms)
  {
    // Check UART commands
    ui_poll();


    // If interval changed
    if (interval_updated)
    {
      interval_updated = false;

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


  Serial.println();
  Serial.println("==============================");
  Serial.println("Hall Effect Sensor");
  Serial.println("==============================");


  // -----------------------------------------------------
  // INITIALIZE HALL SENSOR
  // -----------------------------------------------------

  initializeHallSensor();


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


  Serial.println("Hall Effect Sensor ready.");
}


// =======================================================
// LOOP
// =======================================================

void loop()
{
  // -----------------------------------------------------
  // READ HALL SENSOR
  // -----------------------------------------------------

  int hallState = digitalRead(HALL_PIN);


  // -----------------------------------------------------
  // MAGNET DETECTED
  // -----------------------------------------------------

  if (hallState == LOW)
  {
    Serial.println("Magnet detected!");
  }


  // -----------------------------------------------------
  // NO MAGNET
  // -----------------------------------------------------

  else
  {
    Serial.println("No magnet detected.");
  }


  Serial.println("------------------------");


  // -----------------------------------------------------
  // WAIT FOR NEXT READING
  // -----------------------------------------------------

  waitForInterval();
}
