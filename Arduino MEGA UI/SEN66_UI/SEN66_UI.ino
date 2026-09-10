/*

  Component 10: Sensirion SEN66 Environmental Sensor (WITH UI)
  Target: Arduino Mega 2560
  Library: Sensirion I2C SEN66
  Wiring: I2C (SDA -> Pin 20, SCL -> Pin 21) + EXTERNAL 3.3V POWER
*/

#include <Wire.h>
#include <SensirionI2cSen66.h> 

SensirionI2cSen66 sen66;

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

void setup() {
  Serial.begin(115200); 
  while (!Serial) { delay(100); }

  Wire.begin();
  
  // Pass both the Wire interface and the I2C address (0x6B)
  sen66.begin(Wire, 0x6B);

  Serial.println(F("Initializing SEN66..."));

  // Reset the sensor to ensure a clean state
  uint16_t error = sen66.deviceReset();
  if (error) {
    Serial.println(F("Error resetting SEN66. Check wiring and power!"));
    while(1) delay(10);
  }
  delay(1200); // Wait for the sensor to boot up after reset

  // Start continuous measurement
  error = sen66.startContinuousMeasurement();
  if (error) {
    Serial.println(F("Error starting measurement."));
    while(1) delay(10);
  }
  
  Serial.println(F("SEN66 running."));
}

void loop() {
  uint16_t error;
  
  float massConcentrationPm1p0;
  float massConcentrationPm2p5;
  float massConcentrationPm4p0;
  float massConcentrationPm10p0;
  float ambientHumidity;
  float ambientTemperature;
  float vocIndex;
  float noxIndex;
  uint16_t co2;

  // Read all data at once
  error = sen66.readMeasuredValues(
      massConcentrationPm1p0, massConcentrationPm2p5,
      massConcentrationPm4p0, massConcentrationPm10p0,
      ambientHumidity, ambientTemperature, 
      vocIndex, noxIndex, co2);

  if (error) {
    Serial.println(F("Error reading data from SEN66"));
  } else {
    Serial.println(F("----- SEN66 Readings -----"));
    
    Serial.print(F("PM1.0: ")); Serial.print(massConcentrationPm1p0); Serial.println(F(" ug/m3"));
    Serial.print(F("PM2.5: ")); Serial.print(massConcentrationPm2p5); Serial.println(F(" ug/m3"));
    Serial.print(F("PM4 : ")); Serial.print(massConcentrationPm4p0); Serial.println(F(" ug/m3"));
    Serial.print(F("PM10 : ")); Serial.print(massConcentrationPm10p0); Serial.println(F(" ug/m3"));

    Serial.print(F("Temperature: ")); Serial.print(ambientTemperature); Serial.println(F(" C"));
    Serial.print(F("Relative Humidity: ")); Serial.print(ambientHumidity); Serial.println(F(" %RH"));

    Serial.print(F("VOC : ")); Serial.print(vocIndex); Serial.println();
    Serial.print(F("NOx : ")); Serial.print(noxIndex); Serial.println();
    Serial.print(F("CO2: ")); Serial.print(co2); Serial.println(F(" ppm"));
  
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