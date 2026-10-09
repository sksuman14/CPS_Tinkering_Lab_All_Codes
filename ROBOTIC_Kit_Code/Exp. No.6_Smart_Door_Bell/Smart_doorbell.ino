int const trigPin = 10;
int const echoPin = 12;
int const buzzPin = 4;

void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);   // make sure buzzer starts OFF
  Serial.begin(9600);
}

void loop()
{
  long duration;
  int distance;

  // Proper 10us trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Timeout after 30ms (~5m range) so it doesn't hang if no echo
  duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    Serial.println("No echo - out of range");
    digitalWrite(buzzPin, LOW);  // no object detected -> buzzer off
    delay(60);
    return;
  }

  distance = (duration * 0.034) / 2;

  Serial.print("Distance: ");
  Serial.println(distance);

  if (distance > 0 && distance <= 40)
  {
    digitalWrite(buzzPin, HIGH); // object close -> buzz
  }
  else
  {
    digitalWrite(buzzPin, LOW);  // nothing close -> quiet
  }

  delay(60);
}