#pragma once
#include <Arduino.h>

#define MIRROR_LEFT_PIN_1   3
#define MIRROR_LEFT_PIN_2   7
#define MIRROR_RIGHT_PIN_1  8
#define MIRROR_RIGHT_PIN_2  9

// TODO For now these two defines depend on each other, as the trigger must be more than the move
// else edge cases could happen when the mirror starts moving but the user already shifted away, triggering the back move command too soon
#define MIRROR_TRIGGER_MS 2000  // how long the gear condition must hold before moving
#define MIRROR_MOVE_MS    1500  // motor pulse length

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
