#include <Arduino.h>
#include "CanController.h"
#include "LedController.h"
#include "ColorReceiver.h"

#define CAN_SPEED (500E3) // LOW=33E3, MID=95E3, HIGH=500E3

static const CRGB COLOR_REVERSE = CRGB(0, 255, 0);
static const CRGB COLOR_NEUTRAL = CRGB(255, 255, 0);
static const CRGB COLOR_DRIVE   = CRGB(255, 0, 0);

CanController  canCtrl;
LedController  ledCtrl;
ColorReceiver  colorReceiver;

// Park (and any unrecognised gear value) falls back to the EEPROM-stored user color.
static CRGB colorForGear(Gear gear, const CRGB& storedColor) {
    switch (gear) {
        case GEAR_REVERSE: return COLOR_REVERSE;
        case GEAR_NEUTRAL: return COLOR_NEUTRAL;
        case GEAR_DRIVE:   return COLOR_DRIVE;
        default:           return storedColor;
    }
}

void setup() {
    Serial.begin(250000);
    while (!Serial);

    ledCtrl.begin();
    colorReceiver.begin();

#if RANDOM_CAN == 1
    randomSeed(12345);
    Serial.println("randomCAN Started");
#else
    if (!canCtrl.begin(CAN_SPEED)) {
        Serial.println("Starting CAN failed!");
        while (1);
    }
    Serial.println("CAN RX TX Started");
#endif
}

void loop() {
    canCtrl.update();
    colorReceiver.update();
    ledCtrl.setColor(colorForGear(canCtrl.gear, colorReceiver.getColor()));
    ledCtrl.update(canCtrl.doors, canCtrl.isDark);
}
