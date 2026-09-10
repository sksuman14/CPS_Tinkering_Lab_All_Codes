// IR Sensor Arduino Code

int irSensor = 2;   // IR sensor OUT pin connected to digital pin 2
int led = 13;       // Built-in LED

void setup() {
  pinMode(irSensor, INPUT);
  pinMode(led, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  int sensorValue = digitalRead(irSensor);

  if (sensorValue == LOW) {
    // Object detected
    digitalWrite(led, HIGH);
    Serial.println("Object Detected!");
  } 
  else {
    // No object detected
    digitalWrite(led, LOW);
    Serial.println("No Object");
  }

  delay(200);
}






