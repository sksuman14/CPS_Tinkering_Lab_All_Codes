// Define the pin where the buzzer is connected
const int buzzerPin = 8;

void setup() {
  // Set the buzzer pin as an output
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // Play a tone of 1000 Hz (a standard beep sound)
  tone(buzzerPin, 1000); 
  
  // Wait for 1 second (1000 milliseconds)
  delay(1000); 

  // Stop the tone
  noTone(buzzerPin);     
  
  // Wait for 1 second before looping back and beeping again
  delay(1000); 
}