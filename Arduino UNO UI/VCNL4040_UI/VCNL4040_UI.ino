#include <Wire.h>
#include <Adafruit_VCNL4040.h>
#include <EEPROM.h>

Adafruit_VCNL4040 vcnl4040;


// -------------------------------------------------------
// Command settings
// -------------------------------------------------------
#define CMD_BUF_SIZE 64

char cmd_buf[CMD_BUF_SIZE];
int cmd_len = 0;

int interval_ms = 1000;
bool interval_updated = false;


// -------------------------------------------------------
// EEPROM address
// -------------------------------------------------------
#define EEPROM_INTERVAL_ADDR 0


// -------------------------------------------------------
// Process UART commands
// -------------------------------------------------------
void process_commands(char *buf) {

  // Restart device
  if (strstr(buf, "restartDevice")) {

    Serial.println("Rebooting...");
    delay(50);

    void (*resetFunc)(void) = 0;
    resetFunc();
  }


  // Change interval
  char *p = strstr(buf, "SET_INTERVAL:");

  if (p) {

    int value = atoi(p + strlen("SET_INTERVAL:"));

    if (value > 0) {

      interval_ms = value * 1000;
      interval_updated = true;

      // Save interval so it remains after reset
      EEPROM.put(EEPROM_INTERVAL_ADDR, interval_ms);

      Serial.print("Interval updated to ");
      Serial.print(value);
      Serial.println(" seconds");

    }
    else {

      Serial.println("Invalid interval");
    }
  }


  // Clear command buffer
  cmd_len = 0;
  memset(cmd_buf, 0, sizeof(cmd_buf));
}


// -------------------------------------------------------
// Check for UART commands
// -------------------------------------------------------
void ui_poll() {

  while (Serial.available()) {

    char c = Serial.read();

    // End of command
    if (c == '\n' || c == '\r') {

      if (cmd_len > 0) {

        cmd_buf[cmd_len] = '\0';

        process_commands(cmd_buf);
      }
    }

    // Store command character
    else if (cmd_len < CMD_BUF_SIZE - 1) {

      cmd_buf[cmd_len++] = c;
    }
  }
}


// -------------------------------------------------------
// Setup
// -------------------------------------------------------
void setup() {

  Serial.begin(115200);
  Wire.begin();

  Serial.println("VCNL4040 Sensor");


  // Load previously saved interval
  EEPROM.get(EEPROM_INTERVAL_ADDR, interval_ms);

  // Safety check
  if (interval_ms <= 0) {
    interval_ms = 1000;
  }

  Serial.print("Current interval: ");
  Serial.print(interval_ms / 1000);
  Serial.println(" seconds");


  // Initialize VCNL4040
  if (!vcnl4040.begin()) {

    Serial.println("VCNL4040 not found!");

    while (1);
  }

  Serial.println("VCNL4040 found!");
}


// -------------------------------------------------------
// Loop
// -------------------------------------------------------
void loop() {

  // -----------------------------------------------------
  // Read VCNL4040 values
  // -----------------------------------------------------
  uint16_t proximity = vcnl4040.getProximity();
  uint16_t ambient = vcnl4040.getAmbientLight();
  uint16_t white = vcnl4040.getWhiteLight();
  uint16_t lux = vcnl4040.getLux();


  // -----------------------------------------------------
  // Print all values horizontally
  // -----------------------------------------------------
  Serial.print("VCNL4040:");

  Serial.print("Lux = ");
  Serial.println(lux);


  // -----------------------------------------------------
  // Wait for selected interval
  // -----------------------------------------------------
  int elapsed_ms = 0;

  interval_updated = false;

  while (elapsed_ms < interval_ms) {

    ui_poll();

    if (interval_updated) {
      break;
    }

    delay(100);
    elapsed_ms += 100;
  }
}
