/*
  Component 15: Tipping Bucket Rain Gauge 
  Target: Arduino Mega 2560
  Wiring: Switch Wire 1 -> Pin 2, Switch Wire 2 -> GND
*/

#define RAIN_GAUGE_PIN   2     // Hardware interrupt pin
#define MM_PER_TIP       0.5   // Millimeters of rain per bucket tip
#define DEBOUNCE_TIME_MS 200   // Ignore tips faster than this (mechanical bounce)

// Volatile variables are required when modified inside an ISR
volatile uint32_t tip_count    = 0;
volatile uint32_t last_tip_time = 0;

// -------------------------------------------------------
// Interrupt Service Routine (ISR)
// Called automatically every time the rain gauge pin triggers
// -------------------------------------------------------
void rain_gauge_isr() {  
  uint32_t current_time = millis();

  // Debounce: only count if enough time passed since last tip
  if ((current_time - last_tip_time) > DEBOUNCE_TIME_MS) {
    tip_count++;
    last_tip_time = current_time;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Rain Gauge Test Started...");

  // Configure rain gauge pin as input with internal pull-up
  // (Switch pulls to GND when it closes — pull-up keeps it HIGH normally)
  pinMode(RAIN_GAUGE_PIN, INPUT_PULLUP);

  // Attach interrupt — trigger on FALLING edge
  // (pin goes HIGH->LOW when reed switch closes)
  attachInterrupt(digitalPinToInterrupt(RAIN_GAUGE_PIN), rain_gauge_isr, FALLING);
}

void loop() {
  // Safely read volatile tip_count (disable interrupt briefly to avoid race conditions)
  noInterrupts();
  uint32_t current_tips = tip_count;
  interrupts();

  // Calculate rainfall in mm
  float rainfall_mm = current_tips * MM_PER_TIP;

  // Print data
  Serial.print("Rainfall: ");
  Serial.print(current_tips);
  Serial.print(" tips / ");
  Serial.print(rainfall_mm, 1);
  Serial.println(" mm");

  // Read once every second
  delay(1000); 
}