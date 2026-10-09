#include <SoftwareSerial.h>

// Bluetooth Configuration (HC-05 TX -> D2, HC-05 RX -> D3)
SoftwareSerial Bluetooth(2, 3); 

// Motor Driver Pins
const int IN1 = 5; 
const int IN2 = 6; 
const int IN3 = 7; 
const int IN4 = 8; 

// Ultrasonic Sensor Pins
const int trigPin = 10;
const int echoPin = 12;

// Buzzer Pin
const int buzzerPin = 4; // PUT IT IN D4 if it is external

// Obstacle detection settings
const int safetyDistance = 20; // cm
long duration;
int distance;

// Non-blocking timing for distance checks
unsigned long lastCheckTime = 0;
const int checkInterval = 100; // check distance every 100ms

void setup() {
  // Initialize Hardware Serial Monitor for debugging
  Serial.begin(9600);
  Serial.println("--- System Initialised. Waiting for Bluetooth Command ---");

  // Set motor control pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Set ultrasonic pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Set buzzer pin
  pinMode(buzzerPin, OUTPUT);
  
  // Start Bluetooth serial communication
  Bluetooth.begin(9600);
}

void loop() {
  // Check for incoming mobile Bluetooth data
  if (Bluetooth.available() > 0) {
    char command = Bluetooth.read();
    
    // Ignore hidden mobile carriage return formatting
    if (command != '\n' && command != '\r') {
      Serial.print("Received from phone: [");
      Serial.print(command);
      Serial.println("]");
      
      executeMovement(command);
    }
  }
  
  // Periodically check distance without blocking Bluetooth reads
  unsigned long currentTime = millis();
  if (currentTime - lastCheckTime >= checkInterval) {
    lastCheckTime = currentTime;
    distance = readDistance();
    
    if (distance <= safetyDistance && distance > 0) {
      Serial.print("Obstacle detected at ");
      Serial.print(distance);
      Serial.println(" cm! Buzzer ON");
      digitalWrite(buzzerPin, HIGH); // Buzzer beeps continuously while obstacle is close
    } else {
      digitalWrite(buzzerPin, LOW); // No obstacle, buzzer off
    }
  }
}

// Function to calculate ultrasonic distance
int readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH, 20000); // 20ms timeout to avoid blocking too long
  int cm = duration * 0.034 / 2;
  
  // If reading is out of range or timed out, return a large distance
  if (cm == 0) {
    return 400;
  }
  return cm;
}

void executeMovement(char command) {
  switch (command) {
    case 'U': // Forward
    case 'u': 
      Serial.println("Action: Moving Forward (Continuous)");
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH); 
      digitalWrite(IN4, LOW);
      break;
      
    case 'D': // Backward
    case 'd': 
      Serial.println("Action: Moving Backward (Continuous)");
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, HIGH);
      break;
      
    case 'L': // Turn Left
    case 'l': 
      Serial.println("Action: Turning Left (Continuous)");
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH); 
      digitalWrite(IN4, LOW);
      break;
      
    case 'R': // Turn Right
    case 'r': 
      Serial.println("Action: Turning Right (Continuous)");
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, HIGH);
      break;
      
    case 'C': // Stop
    case 'c': 
    default:
      Serial.println("Action: Stopping Motors");
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, LOW);
      break;
  }
}