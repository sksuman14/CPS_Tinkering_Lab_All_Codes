#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define IR_SENSOR_PIN 10   // Digital pin connected to IR sensor OUT

int objectCount = 0;
int lastState = HIGH;      // Most IR modules output HIGH when no object detected
int currentState;

unsigned long lastDetectTime = 0;
const unsigned long debounceDelay = 50; // ms, prevents double counting

void setup() {
  Serial.begin(9600);
  pinMode(IR_SENSOR_PIN, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true); // halt
  }

  display.clearDisplay();
  updateDisplay();
}

void loop() {
  currentState = digitalRead(IR_SENSOR_PIN);

  // Detect falling edge: HIGH -> LOW means object is blocking the IR beam
  if (lastState == HIGH && currentState == LOW) {
    if (millis() - lastDetectTime > debounceDelay) {
      objectCount++;
      lastDetectTime = millis();
      updateDisplay();
      Serial.print("Object Count: ");
      Serial.println(objectCount);
    }
  }

  lastState = currentState;
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Object");
  display.println("Counter");

  display.setTextSize(3);
  display.setCursor(20, 40);
  display.println(objectCount);

  display.display();
}