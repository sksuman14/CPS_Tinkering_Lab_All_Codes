/*
  Component: IR Obstacle Avoidance Sensor (Normal Version)
  Target: Arduino Mega 2560
  Wiring: OUT/Signal -> Pin 7
*/

const int irPin = 7; 

void setup() {
  Serial.begin(115200);
  pinMode(irPin, INPUT);
  Serial.println(F("IR Sensor Test Started."));
}

void loop() {
  int irState = digitalRead(irPin);
  
  Serial.print(F("IR Sensor Output: "));
  Serial.println(irState);
  
  delay(250); 
}