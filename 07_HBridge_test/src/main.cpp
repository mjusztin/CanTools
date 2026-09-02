#include <Arduino.h>

#define PIN_A1 3
#define PIN_A2 4
#define PIN_B1 5
#define PIN_B2 6

#define STEP_MS 3000

// Cycles through: forward → stop → reverse → stop → repeat
static const struct { uint8_t a1, a2, b1, b2; const char* label; } STEPS[] = {
    { HIGH, LOW,  HIGH, LOW,  "forward" },
    { LOW,  LOW,  LOW,  LOW,  "stop"    },
    { LOW,  HIGH, LOW,  HIGH, "reverse" },
    { LOW,  LOW,  LOW,  LOW,  "stop"    },
};
static const int STEP_COUNT = sizeof(STEPS) / sizeof(STEPS[0]);

static int           step    = 0;
static unsigned long lastMs  = 0;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_A1, OUTPUT);
    pinMode(PIN_A2, OUTPUT);
    pinMode(PIN_B1, OUTPUT);
    pinMode(PIN_B2, OUTPUT);
    Serial.println("H-Bridge test started");
}

void loop() {
    unsigned long now = millis();
    if (now - lastMs < STEP_MS) return;
    lastMs = now;

    auto& s = STEPS[step];
    digitalWrite(PIN_A1, s.a1);
    digitalWrite(PIN_A2, s.a2);
    digitalWrite(PIN_B1, s.b1);
    digitalWrite(PIN_B2, s.b2);
    Serial.print("Step: "); Serial.println(s.label);

    step = (step + 1) % STEP_COUNT;
}
