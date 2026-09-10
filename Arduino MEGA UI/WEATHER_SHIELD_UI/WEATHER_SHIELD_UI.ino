/*
  Component: Weather Shield (BME680 + VEML7700 - WITH UI)
  Target: Arduino Mega 2560
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_VEML7700.h>

// ---- Sensor objects ----
Adafruit_BME680   bme;
Adafruit_VEML7700 veml;

// ---- Sensor readings ----
double t = 0, h = 0, p = 0, l = 0;

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
  char *p_cmd = strstr(buf, "SET_INTERVAL:");
  if (p_cmd) {
    p_cmd += strlen("SET_INTERVAL:");
    while (*p_cmd != '\0' && !isdigit((unsigned char)*p_cmd)) p_cmd++;

    if (isdigit((unsigned char)*p_cmd)) {
      int value = atoi(p_cmd);
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
  Serial.println(F("BME680 + VEML7700 starting..."));

  // Init BME680 at address 0x76
  if (!bme.begin(0x76)) {
    Serial.println(F("BME680 not found! Check wiring."));
    while (1) { delay(10); }
  }
  
  // Configure BME680 parameters
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320 degrees C for 150ms

  // Init VEML7700 at default address 0x10
  if (!veml.begin()) {
    Serial.println(F("VEML7700 not found! Check wiring."));
    while (1) { delay(10); }
  }

  Serial.println(F("BME680 + VEML7700 ready."));
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  // ---- BME680 read ----
  if (bme.performReading()) {
    t = bme.temperature;
    h = bme.humidity;
    p = bme.pressure / 100.0; // Pa to hPa
  } else {
    Serial.println(F("Error: BME680 fetch failed"));
    delay(1000);
    return;
  }

  // ---- VEML7700 read ----
  l = veml.readLux();

  // ---- Print data ----
  Serial.print(F("T:"));  Serial.print(t, 2);
  Serial.print(F(",H:")); Serial.print(h, 2);
  Serial.print(F(",P:")); Serial.print(p, 2);
  Serial.print(F(",L:")); Serial.println(l, 2);

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