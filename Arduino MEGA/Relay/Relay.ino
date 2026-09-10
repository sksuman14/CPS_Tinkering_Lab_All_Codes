/*
  Component 2: 5V Relay Module
  Target: Arduino Mega 2560
*/

// Define the pin connected to the relay signal
const int relayPin = 8; 

void setup() {
  // Initialize the relay pin as an output
  pinMode(relayPin, OUTPUT);
  
  // Start with the relay turned OFF. 
  // NOTE: Many Arduino relay modules are "Active-LOW", 
  // meaning LOW turns them ON and HIGH turns them OFF. 
  // If your relay turns on immediately, change this to HIGH.
  digitalWrite(relayPin, LOW); 
  
  // Optional: Start serial communication for debugging
  Serial.begin(9600);
  Serial.println("Relay Test Started.");
}

void loop() {
  Serial.println("Relay ON");
  digitalWrite(relayPin, HIGH); // Send signal to trigger relay
  delay(3000);                  // Keep it ON for 3 seconds
  
  Serial.println("Relay OFF");
  digitalWrite(relayPin, LOW);  // Send signal to turn off relay
  delay(3000);                  // Keep it OFF for 3 seconds
}