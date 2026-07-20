#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <SoftwareSerial.h>

#define ESP_RX_PIN 4  // Arduino RX ← ESP32 TX (GPIO 17)
#define ESP_TX_PIN 3  // Arduino TX → ESP32 RX (GPIO 16), unused — pin 5 collides with LED_PIN_DRIVER_SIDE, avoid it

class ColorReceiver {
public:
    ColorReceiver();
    void begin(long baud = 9600);
    void update();
    CRGB getColor() const;

private:
    SoftwareSerial _serial;
    CRGB _color;
    char _buf[16];
    int  _ptr;

    void parseLine(char* line);
};
