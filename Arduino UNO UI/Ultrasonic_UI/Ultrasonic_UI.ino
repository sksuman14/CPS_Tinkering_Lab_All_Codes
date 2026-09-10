#include <SoftwareSerial.h>
#include <EEPROM.h>

// --------------------------------------------------
// SENSOR UART
// --------------------------------------------------
#define SENSOR_RX_PIN 10
#define SENSOR_TX_PIN 11

SoftwareSerial sensorSerial(SENSOR_RX_PIN, SENSOR_TX_PIN);

// --------------------------------------------------
// ULTRASONIC ANEMOMETER COMMAND
// --------------------------------------------------
const uint8_t query_command[] = {
  0x65, 0x02, 0x00, 0x00,
  0x00, 0x05, 0xB0, 0x2D
};

#define RESPONSE_LEN 8

// --------------------------------------------------
// EEPROM SETTINGS
// --------------------------------------------------
#define EEPROM_INTERVAL_ADDR 0
#define EEPROM_MAGIC_ADDR    10
#define EEPROM_MAGIC_VALUE   0xA5

// Default interval only if EEPROM has no saved value
#define DEFAULT_INTERVAL_SEC 1

int interval_ms = DEFAULT_INTERVAL_SEC * 1000;

bool interval_updated = false;

// --------------------------------------------------
// UI COMMAND BUFFER
// --------------------------------------------------
#define CMD_BUF_SIZE 64

char cmd_buf[CMD_BUF_SIZE];
int cmd_len = 0;

// --------------------------------------------------
// CRC16 MODBUS
// --------------------------------------------------
uint16_t crc16_modbus(
  const uint8_t *data,
  size_t length
)
{
  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < length; i++)
  {
    crc ^= data[i];

    for (int j = 0; j < 8; j++)
    {
      if (crc & 0x01)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }

  return crc;
}

// --------------------------------------------------
// LOAD INTERVAL FROM EEPROM
// --------------------------------------------------
void loadInterval()
{
  byte magic;

  EEPROM.get(EEPROM_MAGIC_ADDR, magic);

  // Check if a valid interval was previously saved
  if (magic == EEPROM_MAGIC_VALUE)
  {
    int savedInterval;

    EEPROM.get(
      EEPROM_INTERVAL_ADDR,
      savedInterval
    );

    // Validate saved value
    if (savedInterval > 0 &&
        savedInterval <= 86400)
    {
      interval_ms = savedInterval * 1000;

      Serial.print("Saved interval loaded: ");
      Serial.print(savedInterval);
      Serial.println(" seconds");

      return;
    }
  }

  // No valid saved interval
  interval_ms = DEFAULT_INTERVAL_SEC * 1000;

  Serial.print("No saved interval found.");
  Serial.print(" Using default: ");
  Serial.print(DEFAULT_INTERVAL_SEC);
  Serial.println(" second");
}

// --------------------------------------------------
// SAVE INTERVAL TO EEPROM
// --------------------------------------------------
void saveInterval(int seconds)
{
  EEPROM.put(
    EEPROM_INTERVAL_ADDR,
    seconds
  );

  EEPROM.put(
    EEPROM_MAGIC_ADDR,
    (byte)EEPROM_MAGIC_VALUE
  );

  Serial.print("Interval saved to EEPROM: ");
  Serial.print(seconds);
  Serial.println(" seconds");
}

// --------------------------------------------------
// PROCESS UI COMMANDS
// --------------------------------------------------
void process_commands(char *buf)
{
  // ------------------------------------------------
  // RESTART DEVICE
  // ------------------------------------------------
  if (strstr(buf, "restartDevice"))
  {
    Serial.println("Rebooting...");

    delay(100);

    void (*resetFunc)(void) = 0;
    resetFunc();

    return;
  }

  // ------------------------------------------------
  // SET_INTERVAL:<seconds>
  //
  // Example:
  // SET_INTERVAL:5
  // ------------------------------------------------
  char *p = strstr(buf, "SET_INTERVAL:");

  if (p)
  {
    p += strlen("SET_INTERVAL:");

    // Skip non-digit characters
    while (*p != '\0' &&
           !isdigit((unsigned char)*p))
    {
      p++;
    }

    if (isdigit((unsigned char)*p))
    {
      int value = atoi(p);

      // Allow 1 second to 24 hours
      if (value > 0 && value <= 86400)
      {
        interval_ms = value * 1000;

        // Save permanently
        saveInterval(value);

        interval_updated = true;

        Serial.print("Interval updated to ");
        Serial.print(value);
        Serial.println(" seconds");
      }
      else
      {
        Serial.println(
          "Invalid interval."
        );

        Serial.println(
          "Allowed range: 1 - 86400 seconds"
        );
      }
    }
    else
    {
      Serial.println(
        "Invalid interval value."
      );
    }
  }

  // Clear command buffer
  cmd_len = 0;

  memset(
    cmd_buf,
    0,
    sizeof(cmd_buf)
  );
}

// --------------------------------------------------
// UI POLLING
// --------------------------------------------------
void ui_poll()
{
  // Stop SoftwareSerial temporarily
  sensorSerial.end();

  while (Serial.available() > 0)
  {
    char c = Serial.read();

    // End of command
    if (c == '\r' || c == '\n')
    {
      if (cmd_len > 0)
      {
        cmd_buf[cmd_len] = '\0';

        process_commands(cmd_buf);

        cmd_len = 0;

        memset(
          cmd_buf,
          0,
          sizeof(cmd_buf)
        );
      }
    }
    else
    {
      // Store command
      if (cmd_len < CMD_BUF_SIZE - 1)
      {
        cmd_buf[cmd_len++] = c;
      }
    }
  }

  // Restart sensor serial
  sensorSerial.begin(9600);
}

// --------------------------------------------------
// SEND SENSOR QUERY
// --------------------------------------------------
void send_query()
{
  sensorSerial.listen();

  sensorSerial.write(
    query_command,
    sizeof(query_command)
  );

  sensorSerial.flush();

  delay(10);
}

// --------------------------------------------------
// RECEIVE SENSOR RESPONSE
// --------------------------------------------------
bool receive_response(
  uint8_t *buffer,
  size_t length,
  uint32_t timeout
)
{
  size_t received = 0;

  uint32_t startTime = millis();

  while (received < length)
  {
    if (sensorSerial.available())
    {
      buffer[received++] =
        sensorSerial.read();
    }
    else
    {
      if (millis() - startTime > timeout)
      {
        return false;
      }
    }
  }

  return true;
}

// --------------------------------------------------
// READ WIND DATA
// --------------------------------------------------
bool read_wind_data()
{
  uint8_t response[RESPONSE_LEN];

  // Send request
  send_query();

  // Receive response
  if (!receive_response(
        response,
        RESPONSE_LEN,
        1000))
  {
    Serial.println(
      "No complete response from sensor"
    );

    return false;
  }

  // ------------------------------------------------
  // START BYTE CHECK
  // ------------------------------------------------
  if (response[0] != 0x02)
  {
    Serial.println(
      "Invalid response start byte"
    );

    return false;
  }

  // ------------------------------------------------
  // FIXED BYTE CHECK
  // ------------------------------------------------
  if (response[5] != 0x65)
  {
    Serial.println(
      "Invalid sensor data"
    );

    return false;
  }

  // ------------------------------------------------
  // CRC CHECK
  // ------------------------------------------------
  uint16_t calculatedCRC =
    crc16_modbus(response, 6);

  uint16_t receivedCRC =
    (uint16_t)response[6] |
    ((uint16_t)response[7] << 8);

  if (calculatedCRC != receivedCRC)
  {
    Serial.println("CRC error");

    Serial.print("Calculated CRC: 0x");
    Serial.println(
      calculatedCRC,
      HEX
    );

    Serial.print("Received CRC: 0x");
    Serial.println(
      receivedCRC,
      HEX
    );

    return false;
  }

  // ------------------------------------------------
  // WIND SPEED
  // ------------------------------------------------
  float windSpeed =
    response[1] +
    response[2] / 100.0;

  // ------------------------------------------------
  // WIND DIRECTION
  // ------------------------------------------------
  uint16_t windDirection =
    ((uint16_t)response[3] << 8) |
    response[4];

  // ------------------------------------------------
  // PRINT DATA
  // ------------------------------------------------
  Serial.print("Wind speed: ");
  Serial.print(windSpeed, 2);
  Serial.print(" m/s, ");

  Serial.print("Wind direction: ");
  Serial.print(windDirection);
  Serial.println("°");


  return true;
}

// --------------------------------------------------
// SETUP
// --------------------------------------------------
void setup()
{
  // PC / UI serial
  Serial.begin(115200);

  // Sensor serial
  sensorSerial.begin(9600);

  Serial.println();
  Serial.println(
    "Ultrasonic Anemometer"
  );

  Serial.println(
    "Sensor started."
  );

  // Load previously saved interval
  loadInterval();

  Serial.println();
  Serial.println("Commands:");
  Serial.println("SET_INTERVAL:<seconds>");
  Serial.println("restartDevice");

  Serial.println();
}

// --------------------------------------------------
// LOOP
// --------------------------------------------------
void loop()
{
  // Read sensor
  read_wind_data();

  // ------------------------------------------------
  // WAIT FOR NEXT READING
  // ------------------------------------------------
  int elapsed_ms = 0;

  interval_updated = false;

  while (elapsed_ms < interval_ms)
  {
    // Check for UI commands
    ui_poll();

    // If interval changed,
    // start new cycle immediately
    if (interval_updated)
    {
      interval_updated = false;
      break;
    }

    delay(100);

    elapsed_ms += 100;
  }
}
