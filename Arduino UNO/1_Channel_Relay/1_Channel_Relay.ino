#define RELAY_PIN 7

void setup() {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);

  Serial.println("Relay Test Started");
}

void loop() {

  // Relay ON
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("RELAY ON");

  delay(3000);

  // Relay OFF
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("RELAY OFF");

  delay(3000);
}

