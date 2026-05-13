#include "LedController.h"

static const CRGB COLOR_ON  = CRGB(255, 0, 0);
static const CRGB COLOR_OFF = CRGB::Black;

LedController::LedController() {}

void LedController::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN_DRIVER_SIDE,    LED_COLOR_ORDER>(_driverLeds,    LED_COUNT_PER_SIDE);
    FastLED.addLeds<LED_TYPE, LED_PIN_PASSENGER_SIDE, LED_COLOR_ORDER>(_passengerLeds, LED_COUNT_PER_SIDE);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
}

void LedController::update(const DoorState& doors) {
    updateSide(_driverLeds,    doors.driver_front);
    updateSide(_passengerLeds, doors.passenger_front);
    FastLED.show();
}

void LedController::updateSide(CRGB* leds, bool frontDoorOpen) {
    // Leg space (0–9): always on
    for (int i = LED_OFFSET_LEG_SPACE; i < LED_OFFSET_LEG_SPACE + LED_COUNT_LEG_SPACE; i++) {
        leds[i] = COLOR_ON;
    }
    // Door inside (10–14): always on
    for (int i = LED_OFFSET_DOOR_INSIDE; i < LED_OFFSET_DOOR_INSIDE + LED_COUNT_DOOR_INSIDE; i++) {
        leds[i] = COLOR_ON;
    }
    // Under door (15–22): only when front door is open
    for (int i = LED_OFFSET_UNDER_DOOR; i < LED_OFFSET_UNDER_DOOR + LED_COUNT_UNDER_DOOR; i++) {
        leds[i] = frontDoorOpen ? COLOR_ON : COLOR_OFF;
    }
}
