// Ultrasonic Sensor Pins
const int trigPin = 10;
const int echoPin = 12;

// Motor Control Pins (L298N / L293D)
const int leftMotorForward = 5;   // IN1
const int leftMotorBackward = 6;  // IN2
const int rightMotorForward = 7;  // IN3
const int rightMotorBackward = 8; // IN4

// Variables
long duration;
int distance;
const int safetyDistance = 20; // Distance threshold in cm

// Turn timing (tune these for your robot)
const int turnDuration = 400;      // ms for a single left/right turn step
const int doubleTurnDuration = 800; // ms for the "turn right twice" step

void setup() {
  // Sensor Setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Motor Setup
  pinMode(leftMotorForward, OUTPUT);
  pinMode(leftMotorBackward, OUTPUT);
  pinMode(rightMotorForward, OUTPUT);
  pinMode(rightMotorBackward, OUTPUT);
  
  // Start Serial Monitor for debugging
  Serial.begin(9600);
}

void loop() {
  distance = readDistance();
  
  if (distance <= safetyDistance && distance > 0) {
    // Obstacle detected!
    Serial.println("Obstacle detected! Stopping...");
    moveStop();
    delay(200);
    
    // 1. Turn left first
    Serial.println("Turning Left to check path...");
    turnLeft();
    delay(turnDuration);
    moveStop();
    delay(100);
    
    // 2. Check distance after turning left
    int leftCheckDistance = readDistance();
    
    if (leftCheckDistance > safetyDistance) {
      // Path is clear after turning left, continue forward
      Serial.println("Left path clear. Moving forward...");
      moveForward();
      delay(500); // move a bit in this new direction
    } else {
      // Still blocked, turn right twice (to counter the left turn and go right of center)
      Serial.println("Left path blocked. Turning Right (double)...");
      turnRight();
      delay(doubleTurnDuration);
      moveStop();
      delay(100);
      
      Serial.println("Moving forward after right turn...");
      moveForward();
      delay(500);
    }
    
    moveStop();
    delay(200);
    
  } else {
    // Path is clear, move forward
    moveForward();
  }
  
  delay(50); // Small delay between scans
}

// Function to calculate ultrasonic distance
int readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  int cm = duration * 0.034 / 2;
  
  // If reading is out of range, return a large distance
  if (cm == 0) {
    return 400;
  }
  return cm;
}

// Robot Direction Controls
void moveForward() {
  digitalWrite(leftMotorForward, HIGH);
  digitalWrite(leftMotorBackward, LOW);
  digitalWrite(rightMotorForward, HIGH);
  digitalWrite(rightMotorBackward, LOW);
}

void moveBackward() {
  digitalWrite(leftMotorForward, LOW);
  digitalWrite(leftMotorBackward, HIGH);
  digitalWrite(rightMotorForward, LOW);
  digitalWrite(rightMotorBackward, HIGH);
}

void turnLeft() {
  digitalWrite(leftMotorForward, LOW);
  digitalWrite(leftMotorBackward, HIGH);
  digitalWrite(rightMotorForward, HIGH);
  digitalWrite(rightMotorBackward, LOW);
}

void turnRight() {
  digitalWrite(leftMotorForward, HIGH);
  digitalWrite(leftMotorBackward, LOW);
  digitalWrite(rightMotorForward, LOW);
  digitalWrite(rightMotorBackward, HIGH);
}

void moveStop() {
  digitalWrite(leftMotorForward, LOW);
  digitalWrite(leftMotorBackward, LOW);
  digitalWrite(rightMotorForward, LOW);
  digitalWrite(rightMotorBackward, LOW);
}