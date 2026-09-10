#include <Arduino.h>

// =======================================================
// RAIN GAUGE CONFIGURATION
// =======================================================

#define RAIN_GAUGE_PIN 2

// Rainfall per bucket tip
#define MM_PER_TIP 0.5

// Debounce time
#define DEBOUNCE_TIME_MS 200


// =======================================================
// RAIN GAUGE VARIABLES
// =======================================================

volatile uint32_t tip_count = 0;
volatile uint32_t last_tip_time = 0;


// =======================================================
// RAIN GAUGE INTERRUPT
// =======================================================

void rain_gauge_isr()
{
  uint32_t current_time = millis();

  // Ignore mechanical bouncing
  if ((current_time - last_tip_time) > DEBOUNCE_TIME_MS)
  {
    tip_count++;

    last_tip_time = current_time;
  }
}


// =======================================================
// SETUP
// =======================================================

void setup()
{
  Serial.begin(115200);

  Serial.println("Rain Gauge Sensor");

  // Rain gauge reed switch
  // Normally HIGH
  // Becomes LOW when switch closes
  pinMode(RAIN_GAUGE_PIN, INPUT_PULLUP);


  // Trigger whenever rain gauge bucket tips
  attachInterrupt(
    digitalPinToInterrupt(RAIN_GAUGE_PIN),
    rain_gauge_isr,
    FALLING
  );


  Serial.println("Rain gauge ready.");
  Serial.println("Waiting for rain...");
}


// =======================================================
// LOOP
// =======================================================

void loop()
{
  // -----------------------------------------------------
  // Safely read tip count
  // -----------------------------------------------------

  noInterrupts();

  uint32_t current_tips = tip_count;

  interrupts();


  // -----------------------------------------------------
  // Calculate rainfall
  // -----------------------------------------------------

  float rainfall_mm = current_tips * MM_PER_TIP;


  // -----------------------------------------------------
  // Display result
  // -----------------------------------------------------

  Serial.print("Rain Tips: ");
  Serial.println(current_tips);

  Serial.print("Rainfall: ");
  Serial.print(rainfall_mm, 1);
  Serial.println(" mm");


  Serial.println("------------------------");


  delay(1000);
}

