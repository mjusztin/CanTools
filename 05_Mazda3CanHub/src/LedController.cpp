#include "LedController.h"

static const CRGB COLOR_ON   = CRGB(255, 0, 0);
static const CRGB COLOR_OFF  = CRGB::Black;
static const CRGB COLOR_ANIM = CRGB(255, 0, 0);

LedController::LedController() {}

void LedController::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN_DRIVER_SIDE,    LED_COLOR_ORDER>(_driverLeds,    LED_COUNT_DRIVER_SIDE_TOTAL);
    FastLED.addLeds<LED_TYPE, LED_PIN_PASSENGER_SIDE, LED_COLOR_ORDER>(_passengerLeds, LED_COUNT_PASSENGER_SIDE_TOTAL);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
}

void LedController::update(const DoorState& doors) {
    if (_prevDoors.driver_front && !doors.driver_front) {
        _driverAnim.active = true;
        _driverAnim.startMs = millis();
    }
    if (_prevDoors.passenger_front && !doors.passenger_front) {
        _passengerAnim.active = true;
        _passengerAnim.startMs = millis();
    }
    _prevDoors = doors;

    updateSide(_driverLeds,    LED_COUNT_DRIVER_SIDE_LEG_SPACE,    doors.driver_front, _driverAnim);
    updateSide(_passengerLeds, LED_COUNT_PASSENGER_SIDE_LEG_SPACE, doors.passenger_front, _passengerAnim);
    FastLED.show();
}

void LedController::updateSide(CRGB* leds, int legSpaceLedCount, bool frontDoorOpen, SideAnimState& anim) {
    int animPos = -1;
    if (anim.active) {
        unsigned long elapsed = millis() - anim.startMs;
        if (elapsed >= 2000) {
            anim.active = false;
        } else if (elapsed < 1000) {
            animPos = legSpaceLedCount - 1 - (int)((elapsed * legSpaceLedCount) / 1000);
        } else {
            animPos = static_cast<int>(((elapsed - 1000) * legSpaceLedCount) / 1000);
        }
    }

    for (int i = 0; i < legSpaceLedCount; i++) {
        leds[i] = (animPos >= 0) ? (i == animPos ? COLOR_ANIM : COLOR_OFF) : COLOR_ON;
    }
    for (int i = legSpaceLedCount; i < legSpaceLedCount + LED_COUNT_DOOR_INSIDE; i++) {
        leds[i] = COLOR_ON;
    }
    for (int i = legSpaceLedCount + LED_COUNT_DOOR_INSIDE; i < legSpaceLedCount + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR; i++) {
        leds[i] = frontDoorOpen ? COLOR_ON : COLOR_OFF;
    }
}
