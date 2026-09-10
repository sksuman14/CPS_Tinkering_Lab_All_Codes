/*
  Component 12: Reed Switch
  Target: Arduino Uno 
*/

// Define the pin connected to the Reed Switch
const int reedPin = 5; 

void setup() {
  // Use INPUT_PULLUP to keep the pin HIGH when the switch is open
  pinMode(reedPin, INPUT_PULLUP);
  
  Serial.begin(9600);
  Serial.println("Reed Switch Test Started.");
}

void loop() {
  // Read the state of the switch
  int switchState = digitalRead(reedPin);
  
  // Because we connected the other leg to GND, the pin will read LOW 
  // when the magnet closes the switch.
  if (switchState == LOW) {
    Serial.println("Door Closed (Magnet Detected)");
  } else {
    Serial.println("Door Open (No Magnet)");
  }
  
  // Delay to make the Serial Monitor readable
  delay(250); 
}