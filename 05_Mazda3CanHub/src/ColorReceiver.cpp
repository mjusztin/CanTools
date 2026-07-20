#include "ColorReceiver.h"

ColorReceiver::ColorReceiver() : _serial(ESP_RX_PIN, ESP_TX_PIN), _color(255, 0, 0), _ptr(0) {}

void ColorReceiver::begin(long baud) {
    _serial.begin(baud);
}

void ColorReceiver::update() {
    while (_serial.available()) {
        char c = (char)_serial.read();
        if (c == '\n') {
            _buf[_ptr] = '\0';
            parseLine(_buf);
            _ptr = 0;
        } else if (_ptr < (int)sizeof(_buf) - 1) {
            _buf[_ptr++] = c;
        } else {
            _ptr = 0; // overflow — discard and resync
        }
    }
}

CRGB ColorReceiver::getColor() const {
    return _color;
}

void ColorReceiver::parseLine(char* line) {
    int r, g, b;
    if (sscanf(line, "%d,%d,%d", &r, &g, &b) == 3) {
        _color = CRGB(
            (uint8_t)constrain(r, 0, 255),
            (uint8_t)constrain(g, 0, 255),
            (uint8_t)constrain(b, 0, 255)
        );
        Serial.print("Color: ");
        Serial.print(r); Serial.print(",");
        Serial.print(g); Serial.print(",");
        Serial.println(b);
    }
}
