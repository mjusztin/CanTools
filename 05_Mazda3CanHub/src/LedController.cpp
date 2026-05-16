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

void LedController::update(const DoorState& doors) {
    updateSide(_driverLeds,    LED_COUNT_DRIVER_SIDE_LEG_SPACE,    doors.driver_front);
    updateSide(_passengerLeds, LED_COUNT_PASSENGER_SIDE_LEG_SPACE, doors.passenger_front);
    FastLED.show();
}

void LedController::updateSide(CRGB* leds, int legSpaceLedCount, bool frontDoorOpen) {
    for (int i = 0; i < legSpaceLedCount; i++) {
        leds[i] = COLOR_ON;
    }
    for (int i = legSpaceLedCount; i < legSpaceLedCount + LED_COUNT_DOOR_INSIDE; i++) {
        leds[i] = COLOR_ON;
    }
    for (int i = legSpaceLedCount + LED_COUNT_DOOR_INSIDE; i < legSpaceLedCount + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR; i++) {
        leds[i] = frontDoorOpen ? COLOR_ON : COLOR_OFF;
    }
}
