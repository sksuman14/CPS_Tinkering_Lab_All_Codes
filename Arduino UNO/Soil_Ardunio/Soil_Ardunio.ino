#include <SoftwareSerial.h>

// -------------------------------------------------------
// Soil Sensor UART
// Arduino RX (Pin 2) <- Sensor TX
// Arduino TX (Pin 3) -> Sensor RX
// -------------------------------------------------------
#define SENSOR_RX_PIN 2
#define SENSOR_TX_PIN 3

SoftwareSerial sensorSerial(SENSOR_RX_PIN, SENSOR_TX_PIN);

// -------------------------------------------------------
// Send command to sensor
// -------------------------------------------------------
void send_uart_command(const uint8_t *cmd, size_t length)
{
  sensorSerial.write(cmd, length);
  sensorSerial.flush();
}

// -------------------------------------------------------
// Receive data from sensor
// -------------------------------------------------------
bool receive_uart_data(uint8_t *buf, size_t length, uint32_t timeout_ms)
{
  size_t received_len = 0;
  uint32_t start_time = millis();

  while (received_len < length)
  {
    if (sensorSerial.available())
    {
      buf[received_len++] = sensorSerial.read();
    }
    else
    {
      if (millis() - start_time > timeout_ms)
      {
        return false;
      }
    }
  }

  return true;
}

// -------------------------------------------------------
// Modbus CRC16
// -------------------------------------------------------
uint16_t crc16_modbus(const uint8_t *buf, size_t len)
{
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < len; i++)
  {
    crc ^= buf[i];

    for (int j = 0; j < 8; j++)
    {
      if (crc & 0x0001)
      {
        crc = (crc >> 1) ^ 0xA001;
      }
      else
      {
        crc = crc >> 1;
      }
    }
  }

  return crc;
}

// -------------------------------------------------------
// SETUP
// -------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  sensorSerial.begin(9600);

  Serial.println("Soil Sensor Test");
  Serial.println("-----------------");
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop()
{
  // Soil sensor Modbus command
  const uint8_t sensor_cmd[] =
  {
    0x01, 0x03, 0x00, 0x00,
    0x00, 0x08, 0x44, 0x0C
  };

  // Expected response = 21 bytes
  uint8_t rx_buf[21];

  // Send command
  send_uart_command(sensor_cmd, sizeof(sensor_cmd));

  // Receive response
  if (!receive_uart_data(rx_buf, sizeof(rx_buf), 500))
  {
    Serial.println("Sensor response timeout!");
    delay(1000);
    return;
  }

  // -------------------------------------------------------
  // Check response header
  // -------------------------------------------------------
  if (rx_buf[0] != 0x01 || rx_buf[1] != 0x03)
  {
    Serial.print("Invalid response header: ");

    Serial.print(rx_buf[0], HEX);
    Serial.print(" ");
    Serial.println(rx_buf[1], HEX);

    delay(1000);
    return;
  }

  // -------------------------------------------------------
  // Check byte count
  // -------------------------------------------------------
  if (rx_buf[2] != 16)
  {
    Serial.print("Invalid byte count: ");
    Serial.println(rx_buf[2]);

    delay(1000);
    return;
  }

  // -------------------------------------------------------
  // Check CRC
  // -------------------------------------------------------
  uint16_t crc_received =
      (uint16_t)rx_buf[19] |
      ((uint16_t)rx_buf[20] << 8);

  uint16_t crc_calculated =
      crc16_modbus(rx_buf, 19);

  if (crc_received != crc_calculated)
  {
    Serial.print("CRC Error | Received: 0x");
    Serial.print(crc_received, HEX);

    Serial.print(" | Calculated: 0x");
    Serial.println(crc_calculated, HEX);

    delay(1000);
    return;
  }

  // -------------------------------------------------------
  // Parse sensor values
  // -------------------------------------------------------

  uint16_t temp_raw =
      ((uint16_t)rx_buf[3] << 8) | rx_buf[4];

  uint16_t moisture_raw =
      ((uint16_t)rx_buf[5] << 8) | rx_buf[6];

  uint16_t ec_raw =
      ((uint16_t)rx_buf[7] << 8) | rx_buf[8];

  uint16_t salinity_raw =
      ((uint16_t)rx_buf[9] << 8) | rx_buf[10];

  uint16_t nitrogen_raw =
      ((uint16_t)rx_buf[11] << 8) | rx_buf[12];

  uint16_t phosphorus_raw =
      ((uint16_t)rx_buf[13] << 8) | rx_buf[14];

  uint16_t potassium_raw =
      ((uint16_t)rx_buf[15] << 8) | rx_buf[16];

  uint16_t ph_raw =
      ((uint16_t)rx_buf[17] << 8) | rx_buf[18];

  // -------------------------------------------------------
  // Convert values
  // -------------------------------------------------------

  float temperature =
      ((int16_t)temp_raw) / 10.0;

  float moisture =
      moisture_raw / 10.0;

  float ec =
      (float)ec_raw;

  float salinity =
      (float)salinity_raw;

  float nitrogen =
      (float)nitrogen_raw;

  float phosphorus =
      (float)phosphorus_raw;

  float potassium =
      (float)potassium_raw;

  float ph =
      ph_raw / 100.0;

  // -------------------------------------------------------
  // Print sensor data
  // -------------------------------------------------------

  Serial.println();
  Serial.println("Soil Sensor Data");
  Serial.println("----------------");

  Serial.print("Temperature : ");
  Serial.print(temperature, 1);
  Serial.println(" °C");

  Serial.print("Moisture    : ");
  Serial.print(moisture, 1);
  Serial.println(" %");

  Serial.print("EC          : ");
  Serial.println(ec, 0);

  Serial.print("Salinity    : ");
  Serial.println(salinity, 0);

  Serial.print("Nitrogen    : ");
  Serial.println(nitrogen, 0);

  Serial.print("Phosphorus  : ");
  Serial.println(phosphorus, 0);

  Serial.print("Potassium   : ");
  Serial.println(potassium, 0);

  Serial.print("pH          : ");
  Serial.println(ph, 2);

  Serial.println("----------------");

  // Read sensor again after 1 second
  delay(1000);
}
