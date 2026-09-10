/*
  Component: VCNL4040 Ambient Light & Proximity Sensor (WITH UI)
  Target: Arduino Mega 2560
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <Adafruit_VCNL4040.h>

// Initialize the VCNL4040 sensor object
Adafruit_VCNL4040 vcnl = Adafruit_VCNL4040();

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
  Serial.println(F("VCNL4040 Sensor starting..."));

  // Initialize the sensor
  if (!vcnl.begin()) {
    Serial.println(F("Could not find a valid VCNL4040 sensor, check wiring!"));
    while (1) { delay(10); }
  }
  
  Serial.println(F("VCNL4040 Initialized."));
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  // Read the raw ambient light value
  uint16_t raw_als = vcnl.getAmbientLight();
  
  // The VCNL4040 default ambient light resolution is 0.1 lux per bit.
  // Converting this generates the float value to match your print statement.
  float lux = raw_als * 0.1;

  // Print exact matching string format
  Serial.print(F("VCNL4040 : Lux = "));
  Serial.println(lux, 1);

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