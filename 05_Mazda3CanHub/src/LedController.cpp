#include "LedController.h"

static const CRGB COLOR_ON  = CRGB(255, 0, 0);
static const CRGB COLOR_OFF = CRGB::Black;

LedController::LedController() {}

void LedController::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN_DRIVER_SIDE,    LED_COLOR_ORDER>(_driverLeds,    LED_COUNT_DRIVER_SIDE_TOTAL);
    FastLED.addLeds<LED_TYPE, LED_PIN_PASSENGER_SIDE, LED_COLOR_ORDER>(_passengerLeds, LED_COUNT_PASSENGER_SIDE_TOTAL);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
}

void LedController::update(const DoorState& doors, bool isDark) {
    bool lightsEnabled = doors.any_door_open || doors.doors_recently_closed || isDark;
    if (!lightsEnabled) {
        FastLED.clear();
        FastLED.show();
        _prevDoors = doors;
        return;
    }

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
    uint8_t legBrightness = 255;

    if (anim.active) {
        unsigned long elapsed = millis() - anim.startMs;
        if (elapsed >= 3000) {
            anim.active = false;
        } else {
            unsigned long cycleMs = elapsed % 1000;
            legBrightness = cycleMs < 500
                ? (uint8_t)((cycleMs * 255UL) / 500)
                : (uint8_t)(((1000UL - cycleMs) * 255UL) / 500);
        }
    }

    for (int i = 0; i < legSpaceLedCount; i++) {
        leds[i] = COLOR_ON;
        leds[i].nscale8(legBrightness);
    }
    for (int i = legSpaceLedCount; i < legSpaceLedCount + LED_COUNT_DOOR_INSIDE; i++) {
        leds[i] = COLOR_ON;
    }
    for (int i = legSpaceLedCount + LED_COUNT_DOOR_INSIDE; i < legSpaceLedCount + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR; i++) {
        leds[i] = frontDoorOpen ? COLOR_ON : COLOR_OFF;
    }
}
