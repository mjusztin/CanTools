#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "DoorState.h"

#define LED_PIN_DRIVER_SIDE    5
#define LED_PIN_PASSENGER_SIDE 6
#define LED_BRIGHTNESS         255
#define LED_TYPE               WS2815
#define LED_COLOR_ORDER        GRB

#define LED_COUNT_DRIVER_SIDE_LEG_SPACE    10
#define LED_COUNT_PASSENGER_SIDE_LEG_SPACE 11
#define LED_COUNT_DOOR_INSIDE              6
#define LED_COUNT_UNDER_DOOR               17
#define LED_COUNT_DRIVER_SIDE_TOTAL    (LED_COUNT_DRIVER_SIDE_LEG_SPACE    + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR)
#define LED_COUNT_PASSENGER_SIDE_TOTAL (LED_COUNT_PASSENGER_SIDE_LEG_SPACE + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR)

class LedController {
public:
    LedController();
    void begin();
    void update(const DoorState& doors);

private:
    CRGB _driverLeds[LED_COUNT_DRIVER_SIDE_TOTAL];
    CRGB _passengerLeds[LED_COUNT_PASSENGER_SIDE_TOTAL];

    void updateSide(CRGB* leds, int legSpaceLedCount, bool frontDoorOpen);
};
