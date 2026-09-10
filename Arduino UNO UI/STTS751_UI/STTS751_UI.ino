#include <Wire.h>
#include <EEPROM.h>

#define STTS751_ADDRESS 0x48

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

      // Save interval to EEPROM
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

  delay(100);

  // -----------------------------------------------------
  // Load saved interval from EEPROM
  // -----------------------------------------------------
  EEPROM.get(EEPROM_INTERVAL_ADDR, interval_ms);

  // Safety check
  if (interval_ms <= 0) {

    interval_ms = 1000;
  }

  Serial.print("Current interval: ");
  Serial.print(interval_ms / 1000);
  Serial.println(" seconds");


  Serial.println("STTS751 Temperature Sensor");
  Serial.println("STTS751 ready.");
}


// -------------------------------------------------------
// Loop
// -------------------------------------------------------
void loop() {

  // -----------------------------------------------------
  // Tell STTS751 that we want temperature register
  // -----------------------------------------------------
  Wire.beginTransmission(STTS751_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();


  // -----------------------------------------------------
  // Read temperature high byte and low byte
  // -----------------------------------------------------
  Wire.requestFrom(STTS751_ADDRESS, 2);


  if (Wire.available() >= 2) {

    int8_t tempHigh = Wire.read();
    byte tempLow = Wire.read();


    // ---------------------------------------------------
    // STTS751 10-bit temperature
    // ---------------------------------------------------
    float temperature = tempHigh;

    if (tempLow & 0x80) {
      temperature += 0.5;
    }

    if (tempLow & 0x40) {
      temperature += 0.25;
    }


    // ---------------------------------------------------
    // Print temperature
    // ---------------------------------------------------
    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" °C");

  } 
  else {

    Serial.println("STTS751 not found!");
  }


  // -----------------------------------------------------
  // Wait for selected interval
  //
  // UART is checked every 100 ms.
  // -----------------------------------------------------
  int elapsed_ms = 0;

  interval_updated = false;

  while (elapsed_ms < interval_ms) {

    ui_poll();

    // If interval changed, immediately
    // start the next temperature reading.
    if (interval_updated) {
      break;
    }

    delay(100);
    elapsed_ms += 100;
  }
}
