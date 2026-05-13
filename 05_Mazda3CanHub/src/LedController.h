#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "DoorState.h"

#define LED_PIN_DRIVER_SIDE    5
#define LED_PIN_PASSENGER_SIDE 6
#define LED_BRIGHTNESS         255
#define LED_TYPE               WS2815
#define LED_COLOR_ORDER        GRB

#define LED_COUNT_LEG_SPACE   2
#define LED_COUNT_DOOR_INSIDE  2
#define LED_COUNT_UNDER_DOOR   2
#define LED_COUNT_PER_SIDE    (LED_COUNT_LEG_SPACE + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR)

#define LED_OFFSET_LEG_SPACE   0
#define LED_OFFSET_DOOR_INSIDE LED_COUNT_LEG_SPACE
#define LED_OFFSET_UNDER_DOOR  (LED_COUNT_LEG_SPACE + LED_COUNT_DOOR_INSIDE)

class LedController {
public:
    LedController();
    void begin();
    void update(const DoorState& doors);

private:
    CRGB _driverLeds[LED_COUNT_PER_SIDE];
    CRGB _passengerLeds[LED_COUNT_PER_SIDE];

    void updateSide(CRGB* leds, bool frontDoorOpen);
};
