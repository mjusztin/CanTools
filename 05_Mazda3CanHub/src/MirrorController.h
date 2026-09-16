#pragma once
#include <Arduino.h>

#define MIRROR_LEFT_PIN_1   3
#define MIRROR_LEFT_PIN_2   7
#define MIRROR_RIGHT_PIN_1  8
#define MIRROR_RIGHT_PIN_2  9

// A move pulse always runs to completion, so a trigger shorter than the move never cuts it short:
// if the gear flips back mid-move, the opposite move starts once the current one has finished. (At least in theory...)
#define MIRROR_DOWN_TRIGGER_MS 2000  // how long reverse must hold before tilting down
#define MIRROR_UP_TRIGGER_MS   3500  // how long any other gear must hold before tilting back up
#define MIRROR_MOVE_MS         3000  // motor pulse length

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
