#include <SoftwareSerial.h>

// Bluetooth Configuration (HC-05 TX -> D2, HC-05 RX -> D3)
SoftwareSerial Bluetooth(2, 3); 

// Motor Driver Pins
const int IN1 = 5; 
const int IN2 = 6; 
const int IN3 = 7; 
const int IN4 = 8; 

// Buzzer Pin
const int BUZZER = 4;   // Digital pin D4

bool buzzerOn = false;

void setup() {
  Serial.begin(9600);
  Serial.println("--- System Initialised. Waiting for Bluetooth Command ---");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(BUZZER, OUTPUT);        // <-- FIX 1: buzzer pin was never set as output

  digitalWrite(BUZZER, LOW);      // make sure it starts OFF

  Bluetooth.begin(9600);
}

void loop() {
  if (Bluetooth.available() > 0) {
    char command = Bluetooth.read();
    
    if (command != '\n' && command != '\r') {
      Serial.print("Received from phone: [");
      Serial.print(command);
      Serial.println("]");
      
      executeMovement(command);
    }
  }
}

void executeMovement(char command) {
  switch (command) {
    case 'F':
    case 'f': 
      Serial.println("Action: Moving Forward (Continuous)");
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH); 
      digitalWrite(IN4, LOW);
      break;
      
    case 'B':
    case 'b': 
      Serial.println("Action: Moving Backward (Continuous)");
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, HIGH);
      break;
      
    case 'L':
    case 'l': 
      Serial.println("Action: Turning Left (Continuous)");
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH); 
      digitalWrite(IN4, LOW);
      break;
      
    case 'R':
    case 'r': 
      Serial.println("Action: Turning Right (Continuous)");
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, HIGH);
      break;

    case 'H': 
    case 'h':
      buzzerOn = true;
      digitalWrite(BUZZER, HIGH);   // <-- FIX 2: actually turn buzzer on
      Serial.println("Action: Buzzer ON");
      break;
      
    case 'C':
    case 'c':
    default:
      Serial.println("Action: Stopping Motors");
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, LOW);
      buzzerOn = false;
      digitalWrite(BUZZER, LOW);    // <-- FIX 3: actually turn buzzer off
      break;
  }
}