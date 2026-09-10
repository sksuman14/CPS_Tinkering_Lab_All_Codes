#include <Arduino.h>
#include <EEPROM.h>

// -------------------------------------------------------
// Rain Gauge Config
// -------------------------------------------------------
#define RAIN_GAUGE_PIN   2      // Must be pin 2 or 3 on Uno (hardware interrupt pins)
                                // Any GPIO on ESP32
#define MM_PER_TIP       0.5   // Millimeters of rain per bucket tip
#define DEBOUNCE_TIME_MS 200   // Ignore tips faster than this (mechanical bounce)

// -------------------------------------------------------
// EEPROM Config
// -------------------------------------------------------
#define EEPROM_INTERVAL_ADDR 0
#define EEPROM_MAGIC_ADDR    10
#define EEPROM_MAGIC_VALUE   0xA5

// -------------------------------------------------------
// Volatile variables — changed inside ISR
// -------------------------------------------------------
volatile uint32_t tip_count    = 0;
volatile uint32_t last_tip_time = 0;

// -------------------------------------------------------
// ISR — equivalent to rain_gauge_callback() in main.c
// Called every time the rain gauge pin triggers
// -------------------------------------------------------
void rain_gauge_isr() {  uint32_t current_time = millis();

  // Debounce: only count if enough time passed since last tip
  if ((current_time - last_tip_time) > DEBOUNCE_TIME_MS) {
    tip_count++;
    last_tip_time = current_time;
  }
}

// -------------------------------------------------------
// UI State — equivalent to ui.c
// -------------------------------------------------------
#define CMD_BUF_SIZE 64
char     cmd_buf[CMD_BUF_SIZE];
int      cmd_len         = 0;
int      interval_ms     = 1000;  // default 1 second
bool     interval_updated = false;

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
// -------------------------------------------------------
void process_commands(char *buf) {

  // 1. restartDevice
  if (strstr(buf, "restartDevice")) {
    Serial.println("Rebooting...");
    delay(50);
    void (*resetFunc)(void) = 0; // software reset
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
// Polled every 100ms inside the wait loop
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

  Serial.println("Starting Rain Gauge Application...");

  // Configure rain gauge pin as input with internal pull-up
  // (reed switch pulls to GND when it closes — pull-up keeps it HIGH normally)
  pinMode(RAIN_GAUGE_PIN, INPUT_PULLUP);

  // Attach interrupt — trigger on FALLING edge
  // (pin goes HIGH→LOW when reed switch closes, equivalent to GPIO_INT_EDGE_TO_ACTIVE)
  attachInterrupt(digitalPinToInterrupt(RAIN_GAUGE_PIN), rain_gauge_isr, FALLING);

  Serial.println("Rain gauge ready. Waiting for tips...");

  Serial.print("Current interval: ");
  Serial.print(interval_ms / 1000);
  Serial.println(" seconds");
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  // Safely read volatile tip_count (disable interrupt briefly to avoid race)
  noInterrupts();
  uint32_t current_tips = tip_count;
  interrupts();

  // Calculate rainfall in mm
  float rainfall_mm = current_tips * MM_PER_TIP;

  // Print — same format as Nordic version
  Serial.print("Rain Tip Detected! Rainfall: ");
  Serial.print(current_tips);
  Serial.print(" tips / ");
  Serial.print(rainfall_mm, 1);
  Serial.println(" mm");

  // ---- Wait for interval, checking UI every 100ms ----
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

