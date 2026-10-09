#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int trigPin = 10;
const int echoPin = 12;

// Valid measurement range
const float MIN_DISTANCE = 5.0;
const float MAX_DISTANCE = 35.0;

// Timeout tuned for ~60cm max range:
// time = (2 * distance) / speed_of_sound = (2*60)/0.0343 ≈ 3500us
// Add margin -> 5000us (5ms) is plenty, keeps loop fast & responsive
const unsigned long ECHO_TIMEOUT = 5000UL;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

// Single raw reading (noisy)
float getRawDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, ECHO_TIMEOUT);

  if (duration == 0) {
    return -1; // no echo received (out of range / too far / too close)
  }

  // Speed of sound = 0.0343 cm/us, divide by 2 for round trip
  float distance = (duration * 0.0343) / 2.0;
  return distance;
}

// Takes several samples and returns the median for stability
float getDistanceCM() {
  const int SAMPLES = 5;
  float readings[SAMPLES];
  int validCount = 0;

  for (int i = 0; i < SAMPLES; i++) {
    float d = getRawDistanceCM();
    if (d > 0 && d < 400) { // sane hardware range, filtered further below
      readings[validCount++] = d;
    }
    delay(10); // small gap so pulses don't interfere with each other
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

void showDistanceOnOLED(float distance, bool inRange) {
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Distance");

  display.setTextSize(3);
  display.setCursor(0, 30);

  if (inRange) {
    display.print(distance, 1); // one decimal place
    display.setTextSize(2);
    display.println(" cm");
  } else {
    display.setTextSize(2);
    display.setCursor(0, 30);
    display.println("Out of");
    display.println("Range");
  }

  display.display();
}

void loop() {
  float distance = getDistanceCM();

  if (distance > 0 && distance >= MIN_DISTANCE && distance <= MAX_DISTANCE) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    showDistanceOnOLED(distance, true);
  } else {
    Serial.println("Out of range (must be 5-35 cm)");
    showDistanceOnOLED(0, false);
  }

  delay(200); // measurement interval
}