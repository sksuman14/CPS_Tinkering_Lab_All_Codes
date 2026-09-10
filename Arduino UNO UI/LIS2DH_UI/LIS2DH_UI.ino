// id="q7xj3p"
#include <Wire.h>
#include <EEPROM.h>

// LIS2DH I2C address
#define LIS2DH_ADDR 0x18

// -------------------------------------------------------
// UI State
// -------------------------------------------------------
#define CMD_BUF_SIZE 64

char cmd_buf[CMD_BUF_SIZE];
int cmd_len = 0;

int interval_ms = 1000;       // default 1 second
bool interval_updated = false;

// -------------------------------------------------------
// EEPROM Config
// -------------------------------------------------------
#define EEPROM_INTERVAL_ADDR 0
#define EEPROM_MAGIC_ADDR    10
#define EEPROM_MAGIC_VALUE   0xA5

// -------------------------------------------------------
// EEPROM Functions
// -------------------------------------------------------
void save_interval_to_eeprom(int value_seconds)
{
  EEPROM.put(EEPROM_INTERVAL_ADDR, value_seconds);
  EEPROM.update(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
}

void load_interval_from_eeprom()
{
  byte magic = EEPROM.read(EEPROM_MAGIC_ADDR);

  if (magic == EEPROM_MAGIC_VALUE)
  {
    int saved_interval_seconds;

    EEPROM.get(EEPROM_INTERVAL_ADDR, saved_interval_seconds);

    if (saved_interval_seconds > 0)
    {
      interval_ms = saved_interval_seconds * 1000;
    }
  }
}

// -------------------------------------------------------
// process_commands()
// -------------------------------------------------------
void process_commands(char *buf)
{
  // -----------------------------------------------------
  // restartDevice
  // -----------------------------------------------------
  if (strstr(buf, "restartDevice"))
  {
    Serial.println("Rebooting...");
    delay(50);

    void (*resetFunc)(void) = 0;
    resetFunc();

    return;
  }

  // -----------------------------------------------------
  // SET_INTERVAL:<seconds>
  // -----------------------------------------------------
  char *p = strstr(buf, "SET_INTERVAL:");

  if (p)
  {
    p += strlen("SET_INTERVAL:");

    // Skip non-digit characters
    while (*p != '\0' && !isdigit((unsigned char)*p))
    {
      p++;
    }

    if (isdigit((unsigned char)*p))
    {
      int value = atoi(p);

      if (value > 0)
      {
        interval_ms = value * 1000;
        interval_updated = true;

        // Save interval to EEPROM
        save_interval_to_eeprom(value);

        Serial.print("Interval updated to ");
        Serial.print(value);
        Serial.println(" seconds");
      }
      else
      {
        Serial.println("Invalid interval value (must be > 0)");
      }
    }
    else
    {
      Serial.println("Invalid interval value");
    }
  }

  // Reset command buffer
  cmd_len = 0;
  memset(cmd_buf, 0, sizeof(cmd_buf));
}

// -------------------------------------------------------
// ui_poll()
// -------------------------------------------------------
void ui_poll()
{
  while (Serial.available() > 0)
  {
    char c = Serial.read();

    if (c == '\r' || c == '\n')
    {
      if (cmd_len > 0)
      {
        cmd_buf[cmd_len] = '\0';

        process_commands(cmd_buf);

        cmd_len = 0;
        memset(cmd_buf, 0, sizeof(cmd_buf));
      }
    }
    else if (cmd_len < CMD_BUF_SIZE - 1)
    {
      cmd_buf[cmd_len++] = c;
    }
  }
}

// -------------------------------------------------------
// SETUP
// -------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  Wire.begin();

  // Load saved interval from EEPROM
  load_interval_from_eeprom();

  Serial.println("LIS2DH Test");
  Serial.println("----------------");

  // Check WHO_AM_I register
  Wire.beginTransmission(LIS2DH_ADDR);
  Wire.write(0x0F);
  Wire.endTransmission();

  Wire.requestFrom(LIS2DH_ADDR, 1);

  if (Wire.available())
  {
    byte who_am_i = Wire.read();

    Serial.print("WHO_AM_I: 0x");
    Serial.println(who_am_i, HEX);

    if (who_am_i == 0x33)
    {
      Serial.println("LIS2DH detected!");
    }
    else
    {
      Serial.println("Wrong device ID!");
      while (1);
    }
  }
  else
  {
    Serial.println("LIS2DH not detected!");
    Serial.println("Check wiring and I2C address.");
    while (1);
  }

  // Enable X, Y, Z
  // 10 Hz output data rate
  writeRegister(0x20, 0x27);

  // High resolution mode
  // ±2g range
  writeRegister(0x23, 0x08);

  Serial.println("Sensor ready.");

  Serial.print("Current interval: ");
  Serial.print(interval_ms / 1000);
  Serial.println(" seconds");

  Serial.println();
  Serial.println("Commands:");
  Serial.println("restartDevice");
  Serial.println("SET_INTERVAL:<seconds>");
}

// -------------------------------------------------------
// LOOP
// -------------------------------------------------------
void loop()
{
  int16_t x, y, z;

  // Read X, Y, Z registers
  Wire.beginTransmission(LIS2DH_ADDR);

  // 0x28 = OUT_X_L
  // 0x80 = auto increment
  Wire.write(0x28 | 0x80);

  Wire.endTransmission(false);

  Wire.requestFrom(LIS2DH_ADDR, 6);

  if (Wire.available() == 6)
  {
    byte x_low  = Wire.read();
    byte x_high = Wire.read();

    byte y_low  = Wire.read();
    byte y_high = Wire.read();

    byte z_low  = Wire.read();
    byte z_high = Wire.read();

    x = (int16_t)((x_high << 8) | x_low);
    y = (int16_t)((y_high << 8) | y_low);
    z = (int16_t)((z_high << 8) | z_low);

    // 12-bit high-resolution data
    x >>= 4;
    y >>= 4;
    z >>= 4;

    // Approximate conversion for ±2g
    float x_g = x * 0.001f;
    float y_g = y * 0.001f;
    float z_g = z * 0.001f;

    Serial.print("x= ");
    Serial.print(x_g, 3);

    Serial.print(", y= ");
    Serial.print(y_g, 3);

    Serial.print(", z= ");
    Serial.println(z_g, 3);
  }
  else
  {
    Serial.println("Failed to read LIS2DH!");
  }

  // -----------------------------------------------------
  // Wait for interval and check UI every 100ms
  // -----------------------------------------------------
  int elapsed_ms = 0;
  interval_updated = false;

  while (elapsed_ms < interval_ms)
  {
    ui_poll();

    if (interval_updated)
    {
      interval_updated = false;
      break;
    }

    delay(100);
    elapsed_ms += 100;
  }
}

// -------------------------------------------------------
// Write register
// -------------------------------------------------------
void writeRegister(byte reg, byte value)
{
  Wire.beginTransmission(LIS2DH_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
