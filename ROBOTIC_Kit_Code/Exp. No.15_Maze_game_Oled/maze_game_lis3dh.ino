
#include <Wire.h>
#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// ---------------- Display setup ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- Accelerometer setup ----------------
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

const float TILT_THRESHOLD = 3.5; // how hard you must tilt to move (lower = more sensitive)

// If a direction feels reversed on your mounting, flip these to true.
#define INVERT_X false     // for left and right movement
#define INVERT_Y true      // for up and down movement

unsigned long lastMoveTime = 0;
const unsigned long MOVE_INTERVAL = 220; // ms between repeated steps while held tilted

unsigned long lastTapTime = 0;
const unsigned long tapCooldown = 600; // ms

// ---------------- Buzzer ----------------
#define BUZZER_PIN 4

// ---------------- Grid / constant maze (stored in flash, not RAM) ----------------
const uint8_t CELL = 8;
const uint8_t ROWS = 8;
const uint8_t COLS = 16;

// 1 = wall, 0 = open path. PROGMEM keeps this out of SRAM.
const uint8_t maze[ROWS][COLS] PROGMEM = {

 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
 {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},
 {1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1},
 {1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,1},
 {1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1},
 {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1},
 {1,1,1,0,1,1,1,0,1,0,1,1,1,1,0,1},
 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


const int8_t goalRow = 6;
const int8_t goalCol = 14;

// Reads one maze cell out of flash
inline uint8_t mazeAt(uint8_t r, uint8_t c) {
  return pgm_read_byte(&(maze[r][c]));
}

// ---------------- Player / game state ----------------
int8_t  playerRow, playerCol;
uint8_t direction;   // 0=Up, 1=Right, 2=Down, 3=Left
bool    win = false;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  randomSeed(analogRead(A0) + micros());

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;) {} // halt if OLED not found
  }

  if (!lis.begin(0x18)) {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("LIS3DH not found!");
    display.display();
    for (;;) {}
  }
  lis.setRange(LIS3DH_RANGE_4_G);
  lis.setDataRate(LIS3DH_DATARATE_50_HZ);
  lis.setClick(2, 60); // double-click detection, threshold ~60 (tune if needed)

  newGame();
}

void loop() {
  handleTilt();
  handleDoubleTap();
  drawGame();
}

void handleTilt() {
  if (win) return;

  sensors_event_t event;
  lis.getEvent(&event);
  float ax = event.acceleration.x * (INVERT_X ? -1 : 1);
  float ay = event.acceleration.y * (INVERT_Y ? -1 : 1);
  float magX = fabs(ax);
  float magY = fabs(ay);

  if (magX > TILT_THRESHOLD || magY > TILT_THRESHOLD) {
    if (millis() - lastMoveTime > MOVE_INTERVAL) {
      uint8_t dir;
      if (magX >= magY) {
        dir = (ax > 0) ? 1 : 3; // 1=Right, 3=Left
      } else {
        dir = (ay > 0) ? 2 : 0; // 2=Down,  0=Up
      }
      direction = dir;
      moveForward();
      lastMoveTime = millis();
    }
  }
  // No "return to level" requirement - movement simply stops the
  // instant the tilt magnitude drops back below TILT_THRESHOLD.
}

void handleDoubleTap() {
  uint8_t click = lis.getClick();
  if (click & 0x20) {
    if (win && millis() - lastTapTime > tapCooldown) {
      lastTapTime = millis();
      newGame();
    }
  }
}

// Resets the player back to the start of the (fixed) maze
void newGame() {
  playerRow = 1;
  playerCol = 1;
  direction = 1;
  win = false;
}

void moveForward() {
  int8_t newRow = playerRow;
  int8_t newCol = playerCol;

  switch (direction) {
    case 0: newRow--; break; // Up
    case 1: newCol++; break; // Right
    case 2: newRow++; break; // Down
    case 3: newCol--; break; // Left
  }

  if (newRow >= 0 && newRow < ROWS && newCol >= 0 && newCol < COLS) {
    if (mazeAt(newRow, newCol) == 0) {
      playerRow = newRow;
      playerCol = newCol;
      if (playerRow == goalRow && playerCol == goalCol) {
        win = true;
        playWinSound();
      }
    }
  }
}

void playWinSound() {
  tone(BUZZER_PIN, 523, 150);  delay(160);
  tone(BUZZER_PIN, 659, 150);  delay(160);
  tone(BUZZER_PIN, 784, 150);  delay(160);
  tone(BUZZER_PIN, 1047, 300); delay(320);
  noTone(BUZZER_PIN);
}

void drawGame() {
  display.clearDisplay();

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      if (mazeAt(r, c) == 1) {
        display.fillRect(c * CELL, r * CELL, CELL, CELL, SSD1306_WHITE);
      }
    }
  }

  display.drawRect(goalCol * CELL, goalRow * CELL, CELL, CELL, SSD1306_WHITE);
  display.drawPixel(goalCol * CELL + CELL / 2, goalRow * CELL + CELL / 2, SSD1306_WHITE);

  int cx = playerCol * CELL + CELL / 2;
  int cy = playerRow * CELL + CELL / 2;
  drawPlayer(cx, cy, direction);

  if (win) {
    display.fillRect(10, 20, 108, 26, SSD1306_BLACK);
    display.drawRect(10, 20, 108, 26, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(30, 26);
    display.print("YOU WIN!");
    display.setCursor(18, 37);
    //display.print("Double-tap to replay");
  }

  display.display();
}

void drawPlayer(int cx, int cy, uint8_t dir) {
  int s = 3;
  switch (dir) {
    case 0: display.fillTriangle(cx, cy - s, cx - s, cy + s, cx + s, cy + s, SSD1306_WHITE); break;
    case 1: display.fillTriangle(cx + s, cy, cx - s, cy - s, cx - s, cy + s, SSD1306_WHITE); break;
    case 2: display.fillTriangle(cx, cy + s, cx - s, cy - s, cx + s, cy - s, SSD1306_WHITE); break;
    case 3: display.fillTriangle(cx - s, cy, cx + s, cy - s, cx + s, cy + s, SSD1306_WHITE); break;
  }
}
