// #include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME680.h>
// #include <Adafruit_VEML7700.h>

// // ---- Sensor objects ----
// Adafruit_BME680   bme;
// Adafruit_VEML7700 veml;

// // ---- Sensor readings ----
// double t = 0, h = 0, p = 0, l = 0;

// // ---- UI state ----
// #define CMD_BUF_SIZE 64
// char cmd_buf[CMD_BUF_SIZE];
// int  cmd_len        = 0;
// int  interval_ms    = 1000;  // default 1 second
// bool interval_updated = false;

// // -------------------------------------------------------
// // process_commands() — equivalent to ui.c process_commands()
// // Called when a complete line arrives over Serial
// // -------------------------------------------------------
// void process_commands(char *buf) {

//   // 1. restartDevice
//   if (strstr(buf, "restartDevice")) {
//     Serial.println("Rebooting...");
//     delay(50);
//     void (*resetFunc)(void) = 0; // jump to address 0 = software reset
//     resetFunc();
//     return;
//   }

//   // 2. SET_INTERVAL:<seconds>
//   char *p = strstr(buf, "SET_INTERVAL:");
//   if (p) {
//     p += strlen("SET_INTERVAL:");

//     // Skip any non-digit characters
//     while (*p != '\0' && !isdigit((unsigned char)*p)) p++;

//     if (isdigit((unsigned char)*p)) {
//       int value = atoi(p);
//       if (value > 0) {
//         interval_ms     = value * 1000;
//         interval_updated = true;
//         Serial.print("Interval updated to ");
//         Serial.print(value);
//         Serial.println(" seconds");
//       } else {
//         Serial.println("Invalid interval value (must be > 0)");
//       }
//     } else {
//       Serial.println("Invalid interval value");
//     }
//   }

//   // Reset buffer
//   cmd_len = 0;
//   memset(cmd_buf, 0, sizeof(cmd_buf));
// }

// // -------------------------------------------------------
// // ui_poll() — equivalent to uart_cb() in ui.c
// // Called every 100ms inside the wait loop
// // -------------------------------------------------------
// void ui_poll() {
//   while (Serial.available() > 0) {
//     char c = Serial.read();

//     if (c == '\r' || c == '\n') {
//       if (cmd_len > 0) {
//         cmd_buf[cmd_len] = '\0';
//         process_commands(cmd_buf);
//         cmd_len = 0;
//         memset(cmd_buf, 0, sizeof(cmd_buf));
//       }
//     } else if (cmd_len < CMD_BUF_SIZE - 1) {
//       cmd_buf[cmd_len++] = c;
//     }
//   }
// }

// // -------------------------------------------------------
// // SETUP
// // -------------------------------------------------------
// void setup() {
//   Serial.begin(115200);
//   Serial.println("BME680 + VEML7700 starting...");

//   // Init BME680 at address 0x76
//   if (!bme.begin(0x76)) {
//     Serial.println("BME680 not found! Check wiring.");
//     while (1);
//   }
//   bme.setTemperatureOversampling(BME680_OS_8X);
//   bme.setHumidityOversampling(BME680_OS_2X);
//   bme.setPressureOversampling(BME680_OS_4X);
//   bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
//   bme.setGasHeater(320, 150); // 320°C for 150ms

//   // Init VEML7700 at address 0x10
//   if (!veml.begin()) {
//     Serial.println("VEML7700 not found! Check wiring.");
//     while (1);
//   }

//   Serial.println("BME680 + VEML7700 ready.");

// }

// // -------------------------------------------------------
// // LOOP
// // -------------------------------------------------------
// void loop() {
//   // ---- BME680 read ----
//   if (bme.performReading()) {
//     t = bme.temperature;
//     h = bme.humidity;
//     p = bme.pressure / 100.0; // Pa → hPa
//   } else {
//     Serial.println("Error: BME680 fetch failed");
//     delay(1000);
//     return;
//   }

//   // ---- VEML7700 read ----
//   l = veml.readLux();

//   // ---- Print (same format as Nordic version) ----
//   Serial.print("T:");  Serial.print(t, 2);
//   Serial.print(",H:"); Serial.print(h, 2);
//   Serial.print(",P:"); Serial.print(p, 2);
//   Serial.print(",L:"); Serial.println(l, 2);

//   // ---- Wait for interval, checking UI every 100ms ----
//   // Equivalent to the ui_interval_changed() loop in main.c
//   int elapsed_ms = 0;
//   interval_updated = false;

//   while (elapsed_ms < interval_ms) {
//     ui_poll();

//     if (interval_updated) {
//       interval_updated = false;
//       break; // immediately restart with new interval
//     }

//     delay(100);
//     elapsed_ms += 100;
//   }
// }






#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_VEML7700.h>

// ---- Sensor objects ----
Adafruit_BME680   bme;
Adafruit_VEML7700 veml;

// ---- Sensor readings ----
double t = 0, h = 0, p = 0, l = 0;

// ---- UI state ----
#define CMD_BUF_SIZE 64
char cmd_buf[CMD_BUF_SIZE];
int  cmd_len        = 0;
int  interval_ms    = 1000;  // default 1 second
bool interval_updated = false;

// -------------------------------------------------------
// EEPROM Config
// -------------------------------------------------------
#define EEPROM_INTERVAL_ADDR 0
#define EEPROM_MAGIC_ADDR    10
#define EEPROM_MAGIC_VALUE   0xA5

// -------------------------------------------------------
// EEPROM Functions
// -------------------------------------------------------
void save_interval_to_eeprom(int value_seconds) {
  EEPROM.put(EEPROM_INTERVAL_ADDR, value_seconds);
  EEPROM.update(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
}

void load_interval_from_eeprom() {
  byte magic = EEPROM.read(EEPROM_MAGIC_ADDR);

  if (magic == EEPROM_MAGIC_VALUE) {
    int saved_interval_seconds;

    EEPROM.get(EEPROM_INTERVAL_ADDR, saved_interval_seconds);

    if (saved_interval_seconds > 0) {
      interval_ms = saved_interval_seconds * 1000;
    }
  }
}

// -------------------------------------------------------
// process_commands() — equivalent to ui.c process_commands()
// Called when a complete line arrives over Serial
// -------------------------------------------------------
void process_commands(char *buf) {

  // 1. restartDevice
  if (strstr(buf, "restartDevice")) {
    Serial.println("Rebooting...");
    delay(50);
    void (*resetFunc)(void) = 0; // jump to address 0 = software reset
    resetFunc();
    return;
  }

  // 2. SET_INTERVAL:<seconds>
  char *p = strstr(buf, "SET_INTERVAL:");
  if (p) {
    p += strlen("SET_INTERVAL:");

    // Skip any non-digit characters
    while (*p != '\0' && !isdigit((unsigned char)*p)) p++;

    if (isdigit((unsigned char)*p)) {
      int value = atoi(p);
      if (value > 0) {
        interval_ms     = value * 1000;
        interval_updated = true;

        // Save interval to EEPROM
        save_interval_to_eeprom(value);

        Serial.print("Interval updated to ");
        Serial.print(value);
        Serial.println(" seconds");
      } else {
        Serial.println("Invalid interval value (must be > 0)");
      }
    } else {
      Serial.println("Invalid interval value");
    }
  }

  // Reset buffer
  cmd_len = 0;
  memset(cmd_buf, 0, sizeof(cmd_buf));
}

// -------------------------------------------------------
// ui_poll() — equivalent to uart_cb() in ui.c
// Called every 100ms inside the wait loop
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

  // Load saved interval from EEPROM
  load_interval_from_eeprom();

  Serial.println("BME680 + VEML7700 starting...");

  // Init BME680 at address 0x76
  if (!bme.begin(0x76)) {
    Serial.println("BME680 not found! Check wiring.");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320°C for 150ms

  // Init VEML7700 at address 0x10
  if (!veml.begin()) {
    Serial.println("VEML7700 not found! Check wiring.");
    while (1);
  }

  Serial.println("BME680 + VEML7700 ready.");

  Serial.print("Current interval: ");
  Serial.print(interval_ms / 1000);
  Serial.println(" seconds");
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  // ---- BME680 read ----
  if (bme.performReading()) {
    t = bme.temperature;
    h = bme.humidity;
    p = bme.pressure / 100.0; // Pa → hPa
  } else {
    Serial.println("Error: BME680 fetch failed");
    delay(1000);
    return;
  }

  // ---- VEML7700 read ----
  l = veml.readLux();

  // ---- Print (same format as Nordic version) ----
  Serial.print("T:");  Serial.print(t, 2);
  Serial.print(",H:"); Serial.print(h, 2);
  Serial.print(",P:"); Serial.print(p, 2);
  Serial.print(",L:"); Serial.println(l, 2);

  // ---- Wait for interval, checking UI every 100ms ----
  // Equivalent to the ui_interval_changed() loop in main.c
  int elapsed_ms = 0;
  interval_updated = false;

  while (elapsed_ms < interval_ms) {
    ui_poll();

    if (interval_updated) {
      interval_updated = false;
      break; // immediately restart with new interval
    }

    delay(100);
    elapsed_ms += 100;
  }
}

