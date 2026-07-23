#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define ESP_RX_PIN 4  // Arduino RX ← ESP32 TX (GPIO 17)
#define ESP_TX_PIN 3  // Arduino TX → ESP32 RX (GPIO 16), unused for now

#define COLOR_EEPROM_MAGIC      0xCA
#define COLOR_EEPROM_ADDR_MAGIC 0
#define COLOR_EEPROM_ADDR_R     1
#define COLOR_EEPROM_ADDR_G     2
#define COLOR_EEPROM_ADDR_B     3
#define COLOR_WRITE_DELAY_MS    5000

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
    bool          _pendingWrite;
    unsigned long _pendingMs;

    void parseLine(char* line);
    void writeToEeprom();
};
