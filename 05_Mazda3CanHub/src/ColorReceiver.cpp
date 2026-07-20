#include "ColorReceiver.h"

ColorReceiver::ColorReceiver()
    : _serial(ESP_RX_PIN, ESP_TX_PIN), _color(255, 0, 0), _ptr(0),
      _pendingWrite(false), _pendingMs(0) {}

void ColorReceiver::begin(long baud) {
    _serial.begin(baud);

    if (EEPROM.read(COLOR_EEPROM_ADDR_MAGIC) == COLOR_EEPROM_MAGIC) {
        _color = CRGB(
            EEPROM.read(COLOR_EEPROM_ADDR_R),
            EEPROM.read(COLOR_EEPROM_ADDR_G),
            EEPROM.read(COLOR_EEPROM_ADDR_B)
        );
        Serial.print("Color (from EEPROM): ");
    } else {
        Serial.print("Color (default): ");
    }
    Serial.print(_color.r); Serial.print(",");
    Serial.print(_color.g); Serial.print(",");
    Serial.println(_color.b);
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

    if (_pendingWrite && (millis() - _pendingMs >= COLOR_WRITE_DELAY_MS)) {
        writeToEeprom();
        _pendingWrite = false;
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

        _pendingWrite = true;
        _pendingMs = millis();
    }
}

void ColorReceiver::writeToEeprom() {
    EEPROM.update(COLOR_EEPROM_ADDR_MAGIC, COLOR_EEPROM_MAGIC);
    EEPROM.update(COLOR_EEPROM_ADDR_R, _color.r);
    EEPROM.update(COLOR_EEPROM_ADDR_G, _color.g);
    EEPROM.update(COLOR_EEPROM_ADDR_B, _color.b);
    Serial.print("Color saved to EEPROM: ");
    Serial.print(_color.r); Serial.print(",");
    Serial.print(_color.g); Serial.print(",");
    Serial.println(_color.b);
}
