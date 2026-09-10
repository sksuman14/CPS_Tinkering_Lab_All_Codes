/*
  Component 15: Tipping Bucket Rain Gauge (WITH UI)
  Target: Arduino Mega 2560
  Wiring: Switch Wire 1 -> Pin 2, Switch Wire 2 -> GND[cite: 5]
*/

#define RAIN_GAUGE_PIN   2     // Hardware interrupt pin[cite: 5]
#define MM_PER_TIP       0.5   // Millimeters of rain per bucket tip[cite: 5]
#define DEBOUNCE_TIME_MS 200   // Ignore tips faster than this (mechanical bounce)[cite: 5]

// Volatile variables are required when modified inside an ISR[cite: 5]
volatile uint32_t tip_count    = 0;
volatile uint32_t last_tip_time = 0;

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
// Interrupt Service Routine (ISR)
// Called automatically every time the rain gauge pin triggers[cite: 5]
// -------------------------------------------------------
void rain_gauge_isr() {  
  uint32_t current_time = millis();

  // Debounce: only count if enough time passed since last tip[cite: 5]
  if ((current_time - last_tip_time) > DEBOUNCE_TIME_MS) {
    tip_count++;
    last_tip_time = current_time;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Rain Gauge Test Started..."));[cite: 5]

}

void loop() {
  // Safely read volatile tip_count (disable interrupt briefly to avoid race conditions)[cite: 5]
  noInterrupts();
  uint32_t current_tips = tip_count;
  interrupts();

  // Calculate rainfall in mm[cite: 5]
  float rainfall_mm = current_tips * MM_PER_TIP;[cite: 5]

  // Print data[cite: 5]
  Serial.print(F("Rainfall: "));
  Serial.print(current_tips);
  Serial.print(F(" tips / "));
  Serial.print(rainfall_mm, 1);
  Serial.println(F(" mm"));

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