#define HALL_PIN 2

void setup()
{
  Serial.begin(9600);

  pinMode(HALL_PIN, INPUT);

  Serial.println("Hall Effect Sensor");
}

void loop()
{
  int hallState = digitalRead(HALL_PIN);

  if (hallState == LOW)
  {
    Serial.println("Magnet detected!");
  }
  else
  {
    Serial.println("No magnet detected.");
  }

  delay(500);
}