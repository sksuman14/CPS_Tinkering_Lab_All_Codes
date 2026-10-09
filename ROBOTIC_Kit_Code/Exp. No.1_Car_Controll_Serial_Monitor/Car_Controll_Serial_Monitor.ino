
// LEFT motor pair (both left motors wired in parallel to Channel A)
#define IN1 5
#define IN2 6

// RIGHT motor pair (both right motors wired in parallel to Channel B)
#define IN3 7
#define IN4 8

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.begin(9600);
  Serial.println("Ready. Use w/a/s/d to move, x to stop.");
  stopMotors();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    switch (cmd) {
      case 'w': case 'W':
        moveForward();
        Serial.println("Forward");
        break;

      case 's': case 'S':
        moveBackward();
        Serial.println("Backward");
        break;

      case 'a': case 'A':
        turnLeft();
        Serial.println("Left");
        break;

      case 'd': case 'D':
        turnRight();
        Serial.println("Right");
        break;

      case 'x': case 'X':
        stopMotors();
        Serial.println("Stop");
        break;
    }
  }
}

// ---- Motion Functions ----

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  // Left side backward, Right side forward -> pivot left
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  // Right side backward, Left side forward -> pivot right
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}