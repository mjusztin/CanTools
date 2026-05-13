#include <Arduino.h>
#include "CanController.h"
#include "LedController.h"

#define CAN_SPEED (500E3) // LOW=33E3, MID=95E3, HIGH=500E3

CanController canCtrl;
LedController ledCtrl;

void setup() {
    Serial.begin(250000);
    while (!Serial);

    ledCtrl.begin();

#if RANDOM_CAN == 1
    randomSeed(12345);
    Serial.println("randomCAN Started");
#else
    if (!canCtrl.begin(CAN_SPEED)) {
        Serial.println("Starting CAN failed!");
        while (1);
    }
    Serial.println("CAN RX TX Started");
#endif
}

void loop() {
    canCtrl.update();
    ledCtrl.update(canCtrl.doors);
}
