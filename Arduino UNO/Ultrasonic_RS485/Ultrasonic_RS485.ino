#include <SoftwareSerial.h>

#define SENSOR_RX_PIN 10
#define SENSOR_TX_PIN 11

SoftwareSerial sensorSerial(SENSOR_RX_PIN, SENSOR_TX_PIN);

// Command to request data from ultrasonic anemometer
const uint8_t query_command[] = {
  0x65, 0x02, 0x00, 0x00,
  0x00, 0x05, 0xB0, 0x2D
};

#define RESPONSE_LEN 8

// CRC16 Modbus
uint16_t crc16_modbus(const uint8_t *data, size_t length) {

  uint16_t crc = 0xFFFF;

  for (size_t i = 0; i < length; i++) {

    crc ^= data[i];

    for (int j = 0; j < 8; j++) {

      if (crc & 0x01)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }

  return crc;
}

void setup() {

  Serial.begin(115200);
  sensorSerial.begin(9600);

  Serial.println("Ultrasonic Anemometer");
  Serial.println("Sensor started.");
}

void loop() {

  uint8_t response[RESPONSE_LEN];

  // Remove old data
  while (sensorSerial.available()) {
    sensorSerial.read();
  }

  // Send request
  sensorSerial.write(query_command, sizeof(query_command));
  sensorSerial.flush();

  // Wait for response
  unsigned long startTime = millis();
  int index = 0;

  while (index < RESPONSE_LEN &&
         millis() - startTime < 1000) {

    if (sensorSerial.available()) {
      response[index] = sensorSerial.read();
      index++;
    }
  }

  // Check if complete response received
  if (index != RESPONSE_LEN) {

    Serial.println("No complete response from sensor");

    delay(1000);
    return;
  }

  // Check start byte
  if (response[0] != 0x02) {

    Serial.println("Invalid response");

    delay(1000);
    return;
  }

  // Check fixed byte
  if (response[5] != 0x65) {

    Serial.println("Invalid sensor data");

    delay(1000);
    return;
  }

  // Check CRC
  uint16_t calculatedCRC =
    crc16_modbus(response, 6);

  uint16_t receivedCRC =
    (uint16_t)response[6] |
    ((uint16_t)response[7] << 8);

  if (calculatedCRC != receivedCRC) {

    Serial.println("CRC error");

    delay(1000);
    return;
  }

  // Calculate wind speed
  float windSpeed =
    response[1] + response[2] / 100.0;

  // Calculate wind direction
  uint16_t windDirection =
    ((uint16_t)response[3] << 8) |
    response[4];

  // Print sensor values
  Serial.print("Wind speed: ");
  Serial.print(windSpeed, 2);

  Serial.print(", Wind direction: ");
  Serial.println(windDirection);

  delay(1000);
}

