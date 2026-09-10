#include <Wire.h>

// LIS2DH I2C address
#define LIS2DH_ADDR 0x18

void setup()
{
  Serial.begin(115200);
  Wire.begin();

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
  Serial.println();
}

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

  delay(1000);
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
