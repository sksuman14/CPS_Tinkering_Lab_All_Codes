/*
  Component: LIS3DH Accelerometer (WITH UI)
  Target: Arduino Mega 2560
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS3DH.h>

// Initialize the LIS3DH sensor over I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

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
  Serial.println(F("LIS3DH Sensor starting..."));

  // Check standard I2C address 0x18 (or 0x19)
  if (!lis.begin(0x19)) {
    Serial.println(F("Could not find a valid LIS3DH sensor, check wiring!"));
    while (1) { delay(10); }
  }
  
  // Set the measurement range (2G, 4G, 8G, 16G)
  lis.setRange(LIS3DH_RANGE_4_G);
  
  Serial.println(F("LIS3DH Initialized."));
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  // Read sensor events
  sensors_event_t event;
  lis.getEvent(&event);

  // Convert m/s^2 into standard 'g' units to match your required formatting (e.g., 0.98)
  float x_g = event.acceleration.x / SENSORS_GRAVITY_STANDARD;
  float y_g = event.acceleration.y / SENSORS_GRAVITY_STANDARD;
  float z_g = event.acceleration.z / SENSORS_GRAVITY_STANDARD;

  // Print exact matching string format
  Serial.print(F("LIS3DH: x = "));
  Serial.print(x_g, 2);
  Serial.print(F(", y = "));
  Serial.print(y_g, 2);
  Serial.print(F(", z = "));
  Serial.println(z_g, 2);

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