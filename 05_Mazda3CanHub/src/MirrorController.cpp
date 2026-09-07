#include "MirrorController.h"

MirrorController::MirrorController()
    : _current(POS_UP), _desired(POS_UP), _inReverse(false),
      _conditionSinceMs(0), _moving(false), _moveStartMs(0) {}

void MirrorController::begin() {
    pinMode(MIRROR_LEFT_PIN_1,  OUTPUT);
    pinMode(MIRROR_LEFT_PIN_2,  OUTPUT);
    pinMode(MIRROR_RIGHT_PIN_1, OUTPUT);
    pinMode(MIRROR_RIGHT_PIN_2, OUTPUT);
    drive(0);
}

void MirrorController::update(bool inReverse) {
    unsigned long now = millis();

    if (inReverse != _inReverse) {
        _inReverse = inReverse;
        _conditionSinceMs = now;
    }

    // Only a condition held for the full trigger time sets a new target,
    // so a brief shift through reverse never moves the mirrors.
    if (now - _conditionSinceMs >= MIRROR_TRIGGER_MS) {
        _desired = _inReverse ? POS_DOWN : POS_UP;
    }

    if (_moving) {
        // A pulse always runs to completion, even if _desired flipped meanwhile.
        // The opposite move is then picked up on a later call.
        if (now - _moveStartMs >= MIRROR_MOVE_MS) {
            _moving = false;
            drive(0);
        }
        return;
    }

    if (_desired != _current) {
        _current     = _desired;
        _moving      = true;
        _moveStartMs = now;
        drive(_current == POS_DOWN ? 1 : -1);
        Serial.println(_current == POS_DOWN ? "Mirrors: down" : "Mirrors: up");
    }
}

void MirrorController::drive(int8_t direction) {
    uint8_t in1 = (direction > 0) ? HIGH : LOW;
    uint8_t in2 = (direction < 0) ? HIGH : LOW;
    digitalWrite(MIRROR_LEFT_PIN_1,  in1);
    digitalWrite(MIRROR_LEFT_PIN_2,  in2);
    digitalWrite(MIRROR_RIGHT_PIN_1, in1);
    digitalWrite(MIRROR_RIGHT_PIN_2, in2);
}
