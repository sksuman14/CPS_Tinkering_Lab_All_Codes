/*
  Component: SparkFun LIS2DH12 Accelerometer (WITH UI)
  Target: Arduino Mega 2560
  Library: SparkFun LIS2DH12 Arduino Library
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <SparkFun_LIS2DH12.h>

SPARKFUN_LIS2DH12 myIC;

// Conversion factor: 1 mg = 0.00980665 m/s^2
const float MG_TO_MS2 = 0.00980665;

// -------------------------------------------------------
// UI State
// -------------------------------------------------------
#define CMD_BUF_SIZE 64
char cmd_buf[CMD_BUF_SIZE];
int  cmd_len          = 0;
int  interval_ms      = 1000;  // Default 1 second
bool interval_updated = false;

// -------------------------------------------------------
// process_commands()
// -------------------------------------------------------
void process_commands(char *buf) {
  // 1. restartDevice
  if (strstr(buf, "restartDevice")) {
    Serial.println(F("Rebooting..."));
    delay(50);
    void (*resetFunc)(void) = 0; // Software reset
    resetFunc();
    return;
  }

  // 2. SET_INTERVAL:<seconds>
  char *p = strstr(buf, "SET_INTERVAL:");
  if (p) {
    p += strlen("SET_INTERVAL:");
    while (*p != '\0' && !isdigit((unsigned char)*p)) p++;

    if (isdigit((unsigned char)*p)) {
      int value = atoi(p);
      if (value > 0) {
        interval_ms      = value * 1000;
        interval_updated = true;
        Serial.print(F("Interval updated to "));
        Serial.print(value);
        Serial.println(F(" seconds"));
      } else {
        Serial.println(F("Invalid interval value (must be > 0)"));
      }
    } else {
      Serial.println(F("Invalid interval value"));
    }
  }

  // Reset buffer
  cmd_len = 0;
  memset(cmd_buf, 0, sizeof(cmd_buf));
}

// -------------------------------------------------------
// ui_poll()
// -------------------------------------------------------
void ui_poll() {
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\r' || c == '\n') {
      if (cmd_len > 0) {
        cmd_buf[cmd_len] = '\0';
        process_commands(cmd_buf);
        cmd_len = 0;
        memset(cmd_buf, 0, sizeof(cmd_buf));
      }
    } else if (cmd_len < CMD_BUF_SIZE - 1) {
      cmd_buf[cmd_len++] = c;
    }
  }
}

// -------------------------------------------------------
// SETUP
// -------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println(F("SparkFun LIS2DH12 starting..."));

  // Explicitly set the I2C address to 0x18
  if (myIC.begin(0x18) == false) {
    Serial.println(F("Could not find a valid LIS2DH12 sensor at 0x18, check wiring!"));
    while (1) { delay(10); }
  }
  
  Serial.println(F("LIS2DH12 ready."));
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  // Read acceleration values in mg, then convert to m/s^2
  float x = myIC.getX() * MG_TO_MS2;
  float y = myIC.getY() * MG_TO_MS2;
  float z = myIC.getZ() * MG_TO_MS2;
  
  Serial.print(F("LIS2DH: X="));
  Serial.print(x, 2);
  Serial.print(F(", Y="));
  Serial.print(y, 2);
  Serial.print(F(", Z="));
  Serial.println(z, 2);

  // ---- Wait for interval, polling UI every 100ms ----
  int elapsed_ms = 0;
  interval_updated = false;

  while (elapsed_ms < interval_ms) {
    ui_poll();

    if (interval_updated) {
      interval_updated = false;
      break; // restart immediately with new interval
    }

    delay(100);
    elapsed_ms += 100;
  }
}