#include <SoftwareSerial.h>

// -------------------------------------------------------
// SoftwareSerial - Sensor UART (pins 2 and 3)
// -------------------------------------------------------
#define SENSOR_RX_PIN 2   // Arduino RX <- Sensor TX
#define SENSOR_TX_PIN 3   // Arduino TX -> Sensor RX

SoftwareSerial sensorSerial(SENSOR_RX_PIN, SENSOR_TX_PIN);

uint8_t mfg_data[18] = {0};

// -------------------------------------------------------
// UI State
// -------------------------------------------------------
#define CMD_BUF_SIZE 64
char cmd_buf[CMD_BUF_SIZE];
int  cmd_len          = 0;
int  interval_ms      = 1000;  // default 2 seconds (matches original delay(2000))
bool interval_updated = false;

// -------------------------------------------------------
// process_commands() — handles restartDevice + SET_INTERVAL
// -------------------------------------------------------
void process_commands(char *buf) {

  // 1. restartDevice — software reset via jump to address 0
  if (strstr(buf, "restartDevice")) {
    Serial.println(F("Rebooting..."));
    delay(50);
    void (*resetFunc)(void) = 0;
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
// ui_poll() — reads Hardware Serial for incoming commands
// NOTE: SoftwareSerial is used for the sensor (pins 2/3)
//       Hardware Serial (pins 0/1) is used for PC commands
// -------------------------------------------------------
void ui_poll() {
  // Must pause SoftwareSerial listening briefly to read Hardware Serial
  sensorSerial.end();

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

  // Resume SoftwareSerial for sensor communication
  sensorSerial.begin(9600);
}

// -------------------------------------------------------
// Sensor Functions
// -------------------------------------------------------
void send_uart_command(const uint8_t *cmd, size_t length) {
  sensorSerial.listen(); // make sure SoftwareSerial is active
  sensorSerial.write(cmd, length);
  sensorSerial.flush();
}

bool receive_uart_data(uint8_t *buf, size_t length, uint32_t timeout_ms) {
  size_t received_len = 0;
  uint32_t start_time = millis();

  while (received_len < length) {
    if (sensorSerial.available()) {
      buf[received_len++] = sensorSerial.read();
    } else {
      if (millis() - start_time > timeout_ms) {
        return false;
      }
    }
  }
  return true;
}

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
// SETUP
// -------------------------------------------------------
void setup() {
  Serial.begin(115200);          // Hardware UART → PC (debug + commands)
  sensorSerial.begin(9600);      // SoftwareSerial → soil sensor

  Serial.println(F("Soil sensor reader started (Arduino SoftwareSerial)."));
  Serial.println(F("Commands:"));
  Serial.println(F("  restartDevice        -> reboots the Arduino"));
  Serial.println(F("  SET_INTERVAL:<secs>  -> e.g. SET_INTERVAL:5"));
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop() {
  const uint8_t sensor0_cmd[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x08, 0x44, 0x0C};
  uint8_t rx_buf[21];

  send_uart_command(sensor0_cmd, sizeof(sensor0_cmd));

  if (!receive_uart_data(rx_buf, sizeof(rx_buf), 500)) {
    Serial.println(F("receive_uart_data failed (timeout - no response)"));
    // Poll UI even during timeout so commands stay responsive
    int elapsed = 0;
    while (elapsed < 1000) {
      ui_poll();
      delay(100);
      elapsed += 100;
    }
    return;
  }



  // Header checks
  if (rx_buf[0] != 0x01 || rx_buf[1] != 0x03) {
    Serial.print(F("Unexpected header: "));
    Serial.print(rx_buf[0], HEX);
    Serial.print(F(" "));
    Serial.println(rx_buf[1], HEX);
    delay(1000);
    return;
  }
  if (rx_buf[2] != 16) {
    Serial.print(F("Unexpected byte count: "));
    Serial.println(rx_buf[2]);
    delay(1000);
    return;
  }

  // CRC check
  uint16_t crc_recv = (uint16_t)rx_buf[19] | ((uint16_t)rx_buf[20] << 8);
  uint16_t crc_calc = crc16_modbus(rx_buf, 19);
  if (crc_recv != crc_calc) {
    Serial.print(F("CRC mismatch: recv=0x"));
    Serial.print(crc_recv, HEX);
    Serial.print(F(" calc=0x"));
    Serial.println(crc_calc, HEX);
    delay(1000);
    return;
  }

  // Parse 8 registers
  uint16_t temp_raw     = (rx_buf[3]  << 8) | rx_buf[4];
  uint16_t moisture_raw = (rx_buf[5]  << 8) | rx_buf[6];
  uint16_t ec_raw       = (rx_buf[7]  << 8) | rx_buf[8];
  uint16_t sal_raw      = (rx_buf[9]  << 8) | rx_buf[10];
  uint16_t n_raw        = (rx_buf[11] << 8) | rx_buf[12];
  uint16_t p_raw        = (rx_buf[13] << 8) | rx_buf[14];
  uint16_t k_raw        = (rx_buf[15] << 8) | rx_buf[16];
  uint16_t ph_raw       = (rx_buf[17] << 8) | rx_buf[18];

  float temp_c     = ((int16_t)temp_raw) / 10.0f;
  float moisture   = moisture_raw / 10.0f;
  float ec         = (float)ec_raw;
  float sal        = (float)sal_raw;
  float nitrogen   = (float)n_raw;
  float phosphorus = (float)p_raw;
  float potassium  = (float)k_raw;
  float ph_val     = ph_raw / 100.0f;

  // Pack mfg_data
  mfg_data[0]  = 0x59;
  mfg_data[1]  = 0x00;
  mfg_data[2]  = 1;
  mfg_data[3]  = n_raw & 0xFF;
  mfg_data[4]  = (n_raw >> 8) & 0xFF;
  mfg_data[5]  = p_raw & 0xFF;
  mfg_data[6]  = (p_raw >> 8) & 0xFF;
  mfg_data[7]  = k_raw & 0xFF;
  mfg_data[8]  = (k_raw >> 8) & 0xFF;
  mfg_data[9]  = (uint8_t)moisture;
  mfg_data[10] = (uint8_t)temp_c;
  mfg_data[11] = (uint8_t)((temp_c - (uint8_t)temp_c) * 100);
  mfg_data[12] = ec_raw & 0xFF;
  mfg_data[13] = (ec_raw >> 8) & 0xFF;
  mfg_data[14] = (uint8_t)ph_val;
  mfg_data[15] = (uint8_t)((ph_val - (uint8_t)ph_val) * 100);
  mfg_data[16] = sal_raw & 0xFF;
  mfg_data[17] = (sal_raw >> 8) & 0xFF;

  // print_mfg_data();

  // Print parsed data
  Serial.print(F("Soil: N="));   Serial.print(nitrogen, 0);
  Serial.print(F(", P="));       Serial.print(phosphorus, 0);
  Serial.print(F(", K="));       Serial.print(potassium, 0);
  Serial.print(F(", Moist="));   Serial.print(moisture, 1);
  Serial.print(F("%, T="));      Serial.print(temp_c, 1);
  Serial.print(F("C, EC="));     Serial.print(ec, 0);
  Serial.print(F(", pH="));      Serial.print(ph_val, 1);
  Serial.print(F(", Sal="));     Serial.println(sal, 0);

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