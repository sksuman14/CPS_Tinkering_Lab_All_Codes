#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include <EEPROM.h>

Adafruit_LIS3DH lis = Adafruit_LIS3DH();


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


  // Initialize LIS3DH
  if (!lis.begin(0x19)) {

    Serial.println("Could not find LIS3DH!");

    while (1);
  }

  Serial.println("LIS3DH found!");


  // LIS3DH configuration
  lis.setRange(LIS3DH_RANGE_2_G);
  lis.setDataRate(LIS3DH_DATARATE_100_HZ);
}


// -------------------------------------------------------
// Loop
// -------------------------------------------------------
void loop() {

  sensors_event_t event;


  // -----------------------------------------------------
  // Read LIS3DH
  // -----------------------------------------------------
  lis.getEvent(&event);


  // -----------------------------------------------------
  // Get acceleration values
  // -----------------------------------------------------
  float X = event.acceleration.x;
  float Y = event.acceleration.y;
  float Z = event.acceleration.z;


  // -----------------------------------------------------
  // Calculate single tilt angle
  // -----------------------------------------------------
  float tiltAngle = atan2(
    sqrt(X * X + Y * Y),
    abs(Z)
  ) * 180.0 / PI;


  // -----------------------------------------------------
  // Print acceleration and tilt angle
  // -----------------------------------------------------
  Serial.print("X = ");
  Serial.print(X, 2);

  Serial.print("   Y = ");
  Serial.print(Y, 2);

  Serial.print("   Z = ");
  Serial.print(Z, 2);

  Serial.print("   | Tilt angle = ");
  Serial.print(tiltAngle, 1);

  Serial.println(" degrees");


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
    // start the next sensor reading.
    if (interval_updated) {
      break;
    }

    delay(100);
    elapsed_ms += 100;
  }
}
