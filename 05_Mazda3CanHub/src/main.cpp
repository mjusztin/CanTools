#include <Arduino.h>
#include "CanController.h"
#include "LedController.h"
//#include "ColorReceiver.h"
#include "MirrorController.h"

#define CAN_SPEED (500E3) // LOW=33E3, MID=95E3, HIGH=500E3

// Gear colors, hue 160 = blue. CHSV is converted to CRGB once at startup.
static const CRGB COLOR_PARKED  = CRGB(LED_COLOR_PARKED_HSV); // deep, saturated blue, see LedController.h
static const CRGB COLOR_NEUTRAL = CRGB(CHSV(160, 180, 140)); // in-between
static const CRGB COLOR_DRIVE   = CRGB(CHSV(160,  90,  90)); // pale, calm blueish white
static const CRGB COLOR_REVERSE = CRGB(CHSV(160, 255, 255));

CanController  canCtrl;
LedController  ledCtrl;
//ColorReceiver  colorReceiver;
MirrorController mirrorCtrl;

// Park (and any unrecognised gear value) uses the hard-coded parked color.
// The EEPROM-stored user color (ColorReceiver) is disabled for now.
static CRGB colorForGear(Gear gear) {
    switch (gear) {
        case GEAR_REVERSE: return COLOR_REVERSE;
        case GEAR_NEUTRAL: return COLOR_NEUTRAL;
        case GEAR_DRIVE:   return COLOR_DRIVE;
        default:           return COLOR_PARKED;
    }
}

void setup() {
    Serial.begin(250000);
    while (!Serial);

    ledCtrl.begin();
    //colorReceiver.begin();
    mirrorCtrl.begin();

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
    //colorReceiver.update();
    ledCtrl.setColor(colorForGear(canCtrl.gear));
    ledCtrl.update(canCtrl.doors, canCtrl.isDark);
    mirrorCtrl.update(canCtrl.gear == GEAR_REVERSE);
}
