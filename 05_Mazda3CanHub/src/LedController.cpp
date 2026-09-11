#include "LedController.h"

static const CRGB COLOR_OFF = CRGB::Black;

// Boots in the parked color, so the first setColor() starts no fade.
// _baseColor is declared first, hence it is initialised before the three that copy it.
LedController::LedController()
    : _baseColor(LED_COLOR_PARKED_HSV), _color(_baseColor), _fadeFrom(_baseColor), _targetColor(_baseColor) {}

void LedController::setColor(CRGB color) {
    if (color == _baseColor) return;
    _baseColor = color;
    // While a door is open red wins; the new gear color is picked up when the doors close.
    if (!_doorOverride) startFade(_baseColor, LED_COLOR_FADE_MS);
}

void LedController::setPulse(bool enabled) {
    if (enabled == _pulseEnabled) return;
    _pulseEnabled = enabled;
    if (enabled) {
        _pulseStartMs = millis(); // each cycle starts at full brightness, then dips
        _pulsing = true;
    }
    // Switching it off is handled in stepPulse(), at the end of the running cycle.
}

void LedController::startFade(CRGB target, uint16_t durationMs) {
    if (target == _targetColor) return;
    _fadeFrom = _color;   // start from whatever is on the LEDs right now
    _targetColor = target;
    _fadeStartMs = millis();
    _fadeDurationMs = durationMs;
    _fading = true;
}

void LedController::stepColorFade(unsigned long now) {
    if (!_fading) return;

    unsigned long elapsed = now - _fadeStartMs;
    if (elapsed >= _fadeDurationMs) {
        _color = _targetColor;
        _fading = false;
        return;
    }
    fract8 progress = static_cast<fract8>((elapsed * 255UL) / _fadeDurationMs);
    _color = blend(_fadeFrom, _targetColor, progress);
}

// Returns the brightness scale for this frame: 255 while no pulse is running.
uint8_t LedController::stepPulse(unsigned long now) {
    if (!_pulsing) return 255;

    unsigned long elapsed = now - _pulseStartMs;
    if (elapsed >= LED_PULSE_PERIOD_MS) {
        _pulseStartMs += (elapsed / LED_PULSE_PERIOD_MS) * LED_PULSE_PERIOD_MS;
        elapsed %= LED_PULSE_PERIOD_MS;
        if (!_pulseEnabled) { // stop on a cycle boundary, never mid-dip
            _pulsing = false;
            return 255;
        }
    }

    uint8_t theta = static_cast<uint8_t>((elapsed * 255UL) / LED_PULSE_PERIOD_MS);
    uint16_t depth = static_cast<uint16_t>(cos8(theta)) * (255 - LED_PULSE_MIN_SCALE);
    return LED_PULSE_MIN_SCALE + static_cast<uint8_t>(depth / 255);
}

void LedController::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN_DRIVER_SIDE,    LED_COLOR_ORDER>(_driverLeds,    LED_COUNT_DRIVER_SIDE_TOTAL);
    FastLED.addLeds<LED_TYPE, LED_PIN_PASSENGER_SIDE, LED_COLOR_ORDER>(_passengerLeds, LED_COUNT_PASSENGER_SIDE_TOTAL);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
}

void LedController::update(const DoorState& doors, bool isDark) {
    unsigned long now = millis();
    if (now - _lastShowMs < LED_FRAME_INTERVAL_MS) return;
    _lastShowMs = now;

    if (doors.any_door_open != _doorOverride) {
        _doorOverride = doors.any_door_open;
        startFade(_doorOverride ? CRGB(LED_COLOR_DOOR_OPEN_HSV) : _baseColor, LED_DOOR_COLOR_FADE_MS);
    }
    stepColorFade(now);
    uint8_t pulseScale = stepPulse(now);

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

    CRGB frameColor = _color;
    if (pulseScale != 255) frameColor.nscale8(pulseScale);

    updateSide(_driverLeds,    LED_COUNT_DRIVER_SIDE_LEG_SPACE,    doors.driver_front, _driverAnim, frameColor);
    updateSide(_passengerLeds, LED_COUNT_PASSENGER_SIDE_LEG_SPACE, doors.passenger_front, _passengerAnim, frameColor);
    FastLED.show();
}

void LedController::updateSide(CRGB* leds, int legSpaceLedCount, bool frontDoorOpen, SideAnimState& anim, const CRGB& color) {
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
        leds[i] = color;
        leds[i].nscale8(legBrightness);
    }
    for (int i = legSpaceLedCount; i < legSpaceLedCount + LED_COUNT_DOOR_INSIDE; i++) {
        leds[i] = color;
    }
    for (int i = legSpaceLedCount + LED_COUNT_DOOR_INSIDE; i < legSpaceLedCount + LED_COUNT_DOOR_INSIDE + LED_COUNT_UNDER_DOOR; i++) {
        leds[i] = frontDoorOpen ? color : COLOR_OFF;
    }
}
