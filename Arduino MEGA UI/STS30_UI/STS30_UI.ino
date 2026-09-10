/*
  Component: STS30 Temperature Sensor (WITH UI)
  Target: Arduino Mega 2560
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21)
*/

#include <Wire.h>

// Standard I2C address for the STS30 (can be 0x4B if ADDR pin is tied to VDD)
const uint8_t STS30_ADDR = 0x4A; 

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
  
  Serial.println(F("STS30 Sensor Started."));
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  // 1. Request a temperature reading (High Repeatability, Clock Stretching Disabled)
  Wire.beginTransmission(STS30_ADDR);
  Wire.write(0x24); 
  Wire.write(0x00);
  Wire.endTransmission();

  // 2. The STS30 needs roughly 15ms to complete a high-precision measurement
  delay(20);

  // 3. Read the 3 bytes (MSB, LSB, CRC)
  Wire.requestFrom(STS30_ADDR, (uint8_t)3);
  if (Wire.available() >= 3) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t crc = Wire.read(); // CRC is pulled from buffer but ignored for simple reads

    // 4. Calculate Temperature
    uint16_t raw_temp = (msb << 8) | lsb;
    float temp_c = -45.0 + (175.0 * ((float)raw_temp / 65535.0));

    // 5. Print exact matching string format
    Serial.print(F("STS30: Temp: "));
    Serial.print(temp_c, 1);
    Serial.println(F(" C"));
  } else {
    Serial.println(F("STS30: Read Error - Check Wiring."));
  }

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