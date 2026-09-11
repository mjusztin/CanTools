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
#define LED_COUNT_DOOR_INSIDE              6 // Passenger and driver side are the same
#define LED_COUNT_UNDER_DOOR               17 // Passenger and driver side are the same
#define LED_COUNT_DRIVER_SIDE_TOTAL    (LED_COUNT_DRIVER_SIDE_LEG_SPACE    + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR)
#define LED_COUNT_PASSENGER_SIDE_TOTAL (LED_COUNT_PASSENGER_SIDE_LEG_SPACE + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR)

// FastLED.show() disables interrupts for its whole clockless-write duration.
// Throttling refreshes gives SoftwareSerial (ColorReceiver) real gaps to sample RX bits in.
#define LED_FRAME_INTERVAL_MS 20

// Gear changes cross-fade into the new color instead of switching instantly.
#define LED_COLOR_FADE_MS 500

// An open door overrides the gear color with red, with a slower fade in both directions.
#define LED_COLOR_DOOR_OPEN_HSV CHSV(0, 255, 255)
#define LED_DOOR_COLOR_FADE_MS  1000

// Reverse gear adds a slow brightness pulse on top of whatever color is shown.
#define LED_PULSE_PERIOD_MS 3000
#define LED_PULSE_MIN_SCALE 80 // deepest dip of the pulse, 255 = untouched

// Color the LEDs boot in; also used as the parked-gear color in main.cpp.
#define LED_COLOR_PARKED_HSV CHSV(160, 220, 200)

struct SideAnimState {
    bool active = false;
    unsigned long startMs = 0;
};

class LedController {
public:
    LedController();
    void begin();
    void setColor(CRGB color);
    void setPulse(bool enabled);
    void update(const DoorState& doors, bool isDark);

private:
    CRGB _driverLeds[LED_COUNT_DRIVER_SIDE_TOTAL];
    CRGB _passengerLeds[LED_COUNT_PASSENGER_SIDE_TOTAL];
    CRGB _baseColor;    // gear color from setColor(), shown while every door is closed
    CRGB _color;        // currently shown color, may be mid-fade
    CRGB _fadeFrom;     // color the running fade started from
    CRGB _targetColor;  // color the running fade ends on
    unsigned long _fadeStartMs = 0;
    uint16_t _fadeDurationMs = LED_COLOR_FADE_MS;
    bool _fading = false;
    bool _doorOverride = false; // true while red overrides the gear color
    bool _pulseEnabled = false; // pulse requested (reverse gear)
    bool _pulsing = false;      // pulse actually running, outlasts the request by up to one cycle
    unsigned long _pulseStartMs = 0;
    unsigned long _lastShowMs = 0;
    DoorState _prevDoors;
    SideAnimState _driverAnim;
    SideAnimState _passengerAnim;

    void startFade(CRGB target, uint16_t durationMs);
    void stepColorFade(unsigned long now);
    uint8_t stepPulse(unsigned long now);
    void updateSide(CRGB* leds, int legSpaceLedCount, bool frontDoorOpen, SideAnimState& anim);
};
