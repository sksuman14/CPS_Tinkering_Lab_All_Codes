/*
  Component 3: Buzzer (Active or Passive)
  Target: Arduino Mega 2560
*/

// Define the pin connected to the buzzer
const int buzzerPin = 9; 

void setup() {
  // Initialize the buzzer pin as an output
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // tone(pin, frequency_in_hertz)
  // 1000 Hz produces a standard high-pitched beep
  tone(buzzerPin, 1000); 
  delay(1000);             // Beep for 500 milliseconds (half a second)             
  
  noTone(buzzerPin);      
  delay(1000);            // Silence for 1 full second before repeating
}