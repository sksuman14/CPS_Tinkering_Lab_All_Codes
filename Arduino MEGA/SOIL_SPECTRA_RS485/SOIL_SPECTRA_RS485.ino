/*
  Component 14: Soil Spectra NPK Sensor (Modbus UART) - Normal Version
  Target: Arduino Mega 2560
  Wiring: Sensor TX -> Pin 19 (RX1), Sensor RX -> Pin 18 (TX1)
*/

uint8_t mfg_data[18] = {0}; 

// -------------------------------------------------------
// Sensor Functions
// -------------------------------------------------------
void send_uart_command(const uint8_t *cmd, size_t length) { 
  // Clear any leftover data in the buffer BEFORE sending
  while (Serial1.available()) {
    Serial1.read();
  }
  Serial1.write(cmd, length); 
  Serial1.flush(); 
}

bool receive_uart_data(uint8_t *buf, size_t length, uint32_t timeout_ms) { 
  size_t received_len = 0; 
  uint32_t start_time = millis(); 

  while (received_len < length) { 
    if (Serial1.available()) {
      buf[received_len++] = Serial1.read();
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
  Serial.begin(115200);          // Hardware UART0 → PC (debug)
  Serial1.begin(9600);           // Hardware UART1 → Soil Sensor

  Serial.println(F("Soil sensor reader started (Mega Hardware Serial1)."));
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
    delay(1000); 
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

  // Print parsed data
  Serial.print(F("Soil: N="));   Serial.print(nitrogen, 0); 
  Serial.print(F(", P="));       Serial.print(phosphorus, 0); 
  Serial.print(F(", K="));       Serial.print(potassium, 0); 
  Serial.print(F(", Moist="));   Serial.print(moisture, 1); 
  Serial.print(F("%, T="));      Serial.print(temp_c, 1); 
  Serial.print(F("C, EC="));     Serial.print(ec, 0); 
  Serial.print(F(", pH="));      Serial.print(ph_val, 1); 
  Serial.print(F(", Sal="));     Serial.println(sal, 0); 

  // Standard non-blocking wait removed, replaced with simple delay
  delay(1000); 
}