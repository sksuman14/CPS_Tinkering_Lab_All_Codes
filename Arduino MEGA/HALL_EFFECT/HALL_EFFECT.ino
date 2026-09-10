/*
  Component 11: Hall Effect Sensor (Digital)
  Target: Arduino Mega 2560
*/

// Define the pin connected to the Hall Effect sensor
const int hallPin = 6; 

void setup() {
  // We use INPUT_PULLUP in case you are using a bare sensor chip (like the A3144) 
  // rather than a module. This ensures the signal doesn't "float" randomly.
  pinMode(hallPin, INPUT_PULLUP);
  
  Serial.begin(9600);
  Serial.println("Hall Effect Sensor Test Started.");
}

void loop() {
  // Read the state of the sensor
  int magnetDetected = digitalRead(hallPin);
  
  // Most digital Hall Effect sensors are "Active-LOW".
  // This means they output a LOW signal when a magnet is close.
  if (magnetDetected == LOW) {
    Serial.println("Magnet Detected!");
  } else {
    Serial.println("No Magnetic Field.");
  }
  
  // Add a small delay to make the Serial Monitor readable
  delay(250); 
}