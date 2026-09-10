/*
  Component: Ultrasonic Anemometer (WITH UI)
  Target: Arduino Mega 2560
  Wiring: Sensor TX -> Pin 17 (RX2), Sensor RX -> Pin 16 (TX2)
*/

const uint8_t query_command[] = {0x65, 0x02, 0x00, 0x00, 0x00, 0x05, 0xB0, 0x2D};
#define RESPONSE_LEN 8

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
// CRC16 Modbus
// -------------------------------------------------------
uint16_t crc16_modbus(const uint8_t *buf, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= buf[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc = (crc >> 1);
    }
  }
  return crc;
}

// -------------------------------------------------------
// send_query()
// -------------------------------------------------------
void send_query() {
  // Clear any leftover junk in the buffer BEFORE sending the new query
  while (Serial2.available()) {
    Serial2.read();
  }

  // Send the query
  Serial2.write(query_command, sizeof(query_command)); 
  Serial2.flush();
}

// -------------------------------------------------------
// receive_response()
// -------------------------------------------------------
bool receive_response(uint8_t *buf, size_t length, uint32_t timeout_ms) {
  size_t received = 0;
  uint32_t start  = millis();

  while (received < length) {
    if (Serial2.available()) {
      buf[received++] = Serial2.read();
    } else {
      if (millis() - start > timeout_ms) return false;
    }
  }
  return true;
}

// -------------------------------------------------------
// parse_wind()
// -------------------------------------------------------
void parse_wind(uint8_t *r) {
  // Start byte check
  if (r[0] != 0x02) {
    Serial.print(F("Invalid start byte: 0x"));
    Serial.println(r[0], HEX);
    return;
  }

  // Fixed byte check
  if (r[5] != 101) {
    Serial.print(F("Invalid fixed byte: "));
    Serial.println(r[5]);
    return;
  }

  // CRC check
  uint16_t crc_calc = crc16_modbus(r, 6);
  uint16_t crc_recv = (uint16_t)r[6] | ((uint16_t)r[7] << 8);
  if (crc_calc != crc_recv) {
    Serial.print(F("CRC mismatch: calc=0x"));
    Serial.print(crc_calc, HEX);
    Serial.print(F(" recv=0x"));
    Serial.println(crc_recv, HEX);
    return;
  }

  // Parse data
  float    wind_speed     = r[1] + r[2] / 100.0f;
  uint16_t wind_direction = ((uint16_t)r[3] << 8) | r[4];

  // Print results
  Serial.print(F("Wind speed: "));
  Serial.print(wind_speed, 2);
  Serial.print(F(", Wind direction: "));
  Serial.println(wind_direction);
}

// -------------------------------------------------------
// SETUP
// -------------------------------------------------------
void setup() {
  Serial.begin(115200);
  
  // Initialize Hardware Serial2 for the sensor
  Serial2.begin(9600); 

  Serial.println(F("Ultrasonic Anemometer started."));
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  uint8_t response[RESPONSE_LEN];

  send_query();

  if (receive_response(response, RESPONSE_LEN, 1000)) {
    parse_wind(response);
  } else {
    Serial.println(F("Timeout — no response from sensor"));
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