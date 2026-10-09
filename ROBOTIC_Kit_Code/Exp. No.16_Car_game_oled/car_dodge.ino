/*
  CAR DODGE GAME - Arduino Nano
  OLED: 0.96" SSD1306 (I2C, 128x64)
  Motion sensor: LIS3DH (I2C) - tilt left/right to steer

  Wiring (shared I2C bus):
    OLED   VCC->5V/3.3V  GND->GND  SDA->A4  SCL->A5
    LIS3DH VCC->3.3V     GND->GND  SDA->A4  SCL->A5  SDO->GND (addr 0x18)
    BUTTON one leg -> D2, other leg -> GND (internal pullup used, no resistor needed)

  Libraries required (install via Library Manager):
    Adafruit SSD1306
    Adafruit GFX
    Adafruit LIS3DH
    Adafruit Unified Sensor
*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// ---------- OLED SETUP ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- LIS3DH SETUP ----------
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
#define LIS3DH_ADDR 0x18   // use 0x19 if SDO tied to VCC

// ---------- RESET BUTTON ----------
#define RESET_BUTTON_PIN 2   // connect button between D2 and GND
unsigned long lastButtonPress = 0;
const unsigned long buttonDebounce = 250; // ms

// ---------- GAME CONSTANTS ----------
const int LANES = 2;
const int laneWidth = SCREEN_WIDTH / LANES;   // 64 px
const int carW = 12;
const int carH = 10;
const int playerY = SCREEN_HEIGHT - carH - 2;

const int MAX_OBSTACLES = 5;
struct Obstacle {
  int lane;
  float y;
  bool active;
};
Obstacle obstacles[MAX_OBSTACLES];

int playerLane = 0;          // start lane (0 = left, 1 = right)
float obstacleSpeed = 1.6;
unsigned long lastSpawn = 0;
unsigned long spawnInterval = 700;  // ms, decreases as score rises (continuous flow)
const unsigned long minSpawnInterval = 350; // never gets slower than this - keeps obstacles coming

int score = 0;
bool gameOver = false;
unsigned long lastMoveTime = 0;
const unsigned long moveCooldown = 220; // debounce lane changes

// ---------- HELPER: X CENTER OF LANE ----------
int laneCenterX(int lane) {
  return lane * laneWidth + laneWidth / 2;
}

void resetGame() {
  for (int i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = false;
  playerLane = 0;
  obstacleSpeed = 1.6;
  spawnInterval = 700;
  score = 0;
  gameOver = false;
  lastSpawn = millis();
}

bool spawnObstacle() {
  // find a free obstacle slot
  int freeSlot = -1;
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) { freeSlot = i; break; }
  }
  if (freeSlot == -1) return false; // no free slot right now, try again next loop

  // SAFETY CHECK: don't spawn if any active obstacle (in ANY lane) is still
  // near the spawn row. This guarantees the two lanes are never both blocked
  // at the same height, so there's always an escape lane for the player.
  const float minGap = carH * 3.0;
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active && obstacles[i].y > -minGap && obstacles[i].y < minGap) {
      return false; // spawn row still crowded, try again next loop
    }
  }

  obstacles[freeSlot].active = true;
  obstacles[freeSlot].y = -carH;
  obstacles[freeSlot].lane = random(0, LANES);
  return true;
}

void drawCar(int centerX, int y, bool isPlayer) {
  int x = centerX - carW / 2;
  // body (bottom portion of the car)
  display.fillRoundRect(x, y + carH / 3, carW, carH - carH / 3, 2, SSD1306_WHITE);
  // cabin (top portion)
  display.fillRoundRect(x + carW / 4, y, carW - carW / 2, carH / 2, 1, SSD1306_WHITE);
  // wheels (small notches on the sides)
  display.fillRect(x - 1, y + carH / 2, 2, carH / 3, SSD1306_WHITE);
  display.fillRect(x + carW - 1, y + carH / 2, 2, carH / 3, SSD1306_WHITE);
}

void readTiltAndMove() {
  sensors_event_t event;
  lis.getEvent(&event);
  float x = event.acceleration.x; // tilt left/right axis

  const float THRESH = 2.5; // m/s^2, adjust sensitivity here

  if (millis() - lastMoveTime > moveCooldown) {
    if (x > THRESH && playerLane < LANES - 1) {
      playerLane++;
      lastMoveTime = millis();
    } else if (x < -THRESH && playerLane > 0) {
      playerLane--;
      lastMoveTime = millis();
    }
  }
}

bool checkCollision() {
  int px = laneCenterX(playerLane) - carW / 2;
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) continue;
    if (obstacles[i].lane != playerLane) continue;
    int oy = (int)obstacles[i].y;
    // overlap check on Y axis (same lane already matched)
    if (oy + carH >= playerY && oy <= playerY + carH) {
      return true;
    }
  }
  return false;
}

void updateObstacles() {
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) continue;
    obstacles[i].y += obstacleSpeed;
    if (obstacles[i].y > SCREEN_HEIGHT) {
      obstacles[i].active = false;
      score++;
      // difficulty ramps up slowly
      if (obstacleSpeed < 4.5) obstacleSpeed += 0.08;
      if (spawnInterval > minSpawnInterval) spawnInterval -= 15;
    }
  }
}

void drawRoad() {
  // lane dividers (dashed)
  for (int l = 1; l < LANES; l++) {
    int x = l * laneWidth;
    for (int y = 0; y < SCREEN_HEIGHT; y += 8) {
      display.drawFastVLine(x, y, 4, SSD1306_WHITE);
    }
  }
}

void drawGame() {
  display.clearDisplay();
  drawRoad();

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacles[i].active) {
      drawCar(laneCenterX(obstacles[i].lane), (int)obstacles[i].y, false);
    }
  }

  drawCar(laneCenterX(playerLane), playerY, true);

  // score
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 0);
  display.print("Score: ");
  display.print(score);

  display.display();
}

void drawGameOver() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 15);
  display.print("GAME OVER");
  display.setTextSize(1);
  display.setCursor(25, 40);
  display.print("Score: ");
  display.print(score);
  display.setCursor(5, 54);
  display.print("Tilt hard to restart");
  display.display();
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP); // button wired to GND

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 init failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print("Car Dodge Game");
  display.setCursor(10, 40);
  display.print("Tilt to steer!");
  display.display();

  // LIS3DH init
  if (!lis.begin(LIS3DH_ADDR)) {
    Serial.println(F("LIS3DH not found"));
    for (;;);
  }
  lis.setRange(LIS3DH_RANGE_4_G);
  lis.setDataRate(LIS3DH_DATARATE_50_HZ);

  delay(1500);
  resetGame();
}

void loop() {
  // --- RESET BUTTON: works anytime, during play or game over ---
  if (digitalRead(RESET_BUTTON_PIN) == LOW && millis() - lastButtonPress > buttonDebounce) {
    lastButtonPress = millis();
    resetGame();
  }

  if (!gameOver) {
    readTiltAndMove();

    if (millis() - lastSpawn > spawnInterval) {
      if (spawnObstacle()) {
        lastSpawn = millis();
      }
      // if spawn was blocked (row still crowded), we simply try again
      // on the next loop iteration instead of waiting a full interval
    }

    updateObstacles();

    if (checkCollision()) {
      gameOver = true;
    }

    drawGame();
  } else {
    drawGameOver();

    // restart on a strong shake/tilt
    sensors_event_t event;
    lis.getEvent(&event);
    float mag = sqrt(event.acceleration.x * event.acceleration.x +
                      event.acceleration.y * event.acceleration.y);
    if (mag > 12.0) {
      delay(300);
      resetGame();
    }
  }

  delay(30); // ~30fps loop pacing
}
