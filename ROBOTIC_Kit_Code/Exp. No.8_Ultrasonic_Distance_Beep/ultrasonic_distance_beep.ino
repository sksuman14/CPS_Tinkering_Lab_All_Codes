const int trigPin = 10;
const int echoPin = 12;
const int buzzerPin = 4;

float lastRefDistance = -1;   // reference distance for comparison
const float DECREASE_STEP = 10.0; // cm threshold to trigger beep

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);\
  digitalWrite(buzzerPin, LOW);
}

// Single raw reading (noisy)
float getRawDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo pulse duration (timeout ~30ms ~ 5m range)
  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return -1; // no echo received (out of range)
  }

  // Speed of sound = 0.0343 cm/us, divide by 2 for round trip
  float distance = (duration * 0.0343) / 2.0;
  return distance;
}

// Takes several samples and returns the median, which is far more
// stable than a single raw reading (rejects random spikes/dropouts)
float getDistanceCM() {
  const int SAMPLES = 5;
  float readings[SAMPLES];
  int validCount = 0;

  for (int i = 0; i < SAMPLES; i++) {
    float d = getRawDistanceCM();
    if (d > 0 && d < 400) { // sane range, HC-SR04 max ~400cm
      readings[validCount++] = d;
    }
    delay(15); // small gap so pulses don't interfere with each other
  }

  if (validCount == 0) return -1;

  // simple insertion sort (small array, fine for 5 elements)
  for (int i = 1; i < validCount; i++) {
    float key = readings[i];
    int j = i - 1;
    while (j >= 0 && readings[j] > key) {
      readings[j + 1] = readings[j];
      j--;
    }
    readings[j + 1] = key;
  }

  return readings[validCount / 2]; // median value
}

void beep() {
  digitalWrite(buzzerPin, HIGH);
  delay(150);
  digitalWrite(buzzerPin, LOW);
}

void loop() {
  float distance = getDistanceCM();

  if (distance > 0) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (lastRefDistance < 0) {
      // First valid reading, set as reference
      lastRefDistance = distance;
    } else if (distance < lastRefDistance) {
      // Object getting closer
      if (lastRefDistance - distance >= DECREASE_STEP) {
        beep();
        Serial.println(">>> Beep! Distance decreased by 10 cm <<<");
        lastRefDistance = distance; // update reference
      }
    } else if (distance > lastRefDistance) {
      // Object moving away, reset reference so future decreases
      // are measured from the current farther point
      lastRefDistance = distance;
    }
  } else {
    Serial.println("Out of range");
  }

  delay(200); // measurement interval
}
