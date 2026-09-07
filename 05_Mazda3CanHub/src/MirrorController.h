#pragma once
#include <Arduino.h>

#define MIRROR_LEFT_PIN_1   3
#define MIRROR_LEFT_PIN_2   7
#define MIRROR_RIGHT_PIN_1  8
#define MIRROR_RIGHT_PIN_2  9

#define MIRROR_TRIGGER_MS 1000  // how long the gear condition must hold before moving
#define MIRROR_MOVE_MS     500  // motor pulse length

class MirrorController {
public:
    MirrorController();
    void begin();
    void update(bool inReverse);

private:
    enum Position : uint8_t { POS_UP, POS_DOWN };

    Position      _current;
    Position      _desired;
    bool          _inReverse;
    unsigned long _conditionSinceMs;
    bool          _moving;
    unsigned long _moveStartMs;

    void drive(int8_t direction); // +1 down, -1 up, 0 coast
};
