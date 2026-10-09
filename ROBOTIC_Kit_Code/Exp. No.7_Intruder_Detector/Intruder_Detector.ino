#define TriggerPin 10
#define EchoPin 12
#define buzzer 4

long Duration;
float Distance;

bool alarmActive = false;

const float ENTER_THRESHOLD = 15;  // trigger alarm when object closer than this
const float EXIT_THRESHOLD  = 25;  // only clear alarm when object farther than this

float getDistance() {
    digitalWrite(TriggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(TriggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(TriggerPin, LOW);

    long duration = pulseIn(EchoPin, HIGH, 30000);
    if (duration == 0) return -1;
    return (duration * 0.0343) / 2.0;
}

void setup() {
    Serial.begin(9600);
    pinMode(TriggerPin, OUTPUT);
    pinMode(EchoPin, INPUT);
    pinMode(buzzer, OUTPUT);
}

void loop() {
    Distance = getDistance();

    if (Distance > 0) {
        if (!alarmActive && Distance <= ENTER_THRESHOLD) {
            alarmActive = true;   // object just came close -> trigger immediately
        }
        else if (alarmActive && Distance > EXIT_THRESHOLD) {
            alarmActive = false; // object clearly moved away -> clear alarm
        }
    }
    // if Distance == -1 (no echo), we deliberately do nothing —
    // don't clear the alarm just because of one missed reading

    if (alarmActive) {
        Serial.print("INTRUDER! At ");
        Serial.print(Distance);
        Serial.println(" cm");
        digitalWrite(buzzer, HIGH);
    } else {
        Serial.print("Distance: ");
        Serial.print(Distance);
        Serial.println(" cm");
        digitalWrite(buzzer, LOW);
    }

    delay(100);  // fast loop = quick initial detection
}