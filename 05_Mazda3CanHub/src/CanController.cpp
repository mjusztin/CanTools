#include "CanController.h"
#include <SPI.h>

CanController* CanController::_instance = nullptr;

static ACAN2515Tiny gCan(MCP2515_CS, SPI, MCP2515_INT);
static void canISR() { gCan.isr(); }

CanController::CanController() {}

bool CanController::begin(long canSpeed) {
    _instance = this;
#if RANDOM_CAN == 0
    SPI.begin();

    ACAN2515TinySettings settings(MCP2515_QUARTZ, (uint32_t)canSpeed);
    // Defaults are 32 RX + 16 TX frames; at 16 bytes per CANMessage that would be
    // ~768 B of the Nano's 2 KB, on top of FastLED's ~200 B of LED buffers.
    settings.mReceiveBufferSize  = 8;
    settings.mTransmitBufferSize = 4;

    // Exact-match on every ID: the mask marks all 11 identifier bits as significant.
    // The two trailing bytes filter the first two data bytes, 0 = don't care.
    const ACAN2515Mask rxm0 = standard2515Mask(0x7FF, 0, 0);
    const ACAN2515AcceptanceFilter filters[] = {
        {standard2515Filter(0x09A, 0, 0), onLightSensorFrame},
        {standard2515Filter(0x43E, 0, 0), onDoorFrame},
    };
    // Filter-count rule enforced by the library: the single-mask begin() accepts
    // 1-2 filters, the two-mask overload accepts 3-6. Adding a third ID therefore
    // means switching to begin(settings, canISR, rxm0, rxm1, filters, count).
    const uint16_t err = gCan.begin(settings, canISR, rxm0, filters,
                                    sizeof(filters) / sizeof(filters[0]));
    if (err != 0) {
        Serial.print("MCP2515 begin error 0x");
        Serial.println(err, HEX);
        return false;
    }
#endif
    return true;
}

void CanController::update() {
    while (gCan.dispatchReceivedMessage()); // dispatches one frame per call
    rxCallback();
    printDoorChanges();
#if RANDOM_CAN == 1
    CANsimulate();
    delay(100);
#endif
}

//------------------------------------------------------------------------------
// CAN packet simulator
#if RANDOM_CAN == 1
void CanController::CANsimulate() {
    packet_t txPacket;
    long sampleIdList[] = {0x110, 0x18DAF111, 0x23A, 0x257, 0x412F1A1, 0x601, 0x18EA0C11};
    int idIndex = random(sizeof(sampleIdList) / sizeof(sampleIdList[0]));
    int sampleData[] = {0xA, 0x1B, 0x2C, 0x3D, 0x4E, 0x5F, 0xA0, 0xB1};

    txPacket.id  = sampleIdList[idIndex];
    txPacket.ide = txPacket.id > 0x7FF ? 1 : 0;
    txPacket.rtr = 0;
    txPacket.dlc = random(1, 9);

    for (int i = 0; i < txPacket.dlc; i++) {
        if (random(4) == 0) sampleData[i] = random(256);
        txPacket.dataArray[i] = sampleData[i];
    }
    printPacket(&txPacket);
}
#endif

//------------------------------------------------------------------------------
// CAN RX, TX
void CanController::onLightSensorFrame(const CANMessage& frame) {
    if (!_instance) return;
    _instance->isDark = (frame.data[6] & 0x04) != 0;
}

void CanController::onDoorFrame(const CANMessage& frame) {
    if (!_instance) return;
    DoorState& d = _instance->doors;
    d.any_door_open         = (frame.data[3] == 0x60);
    d.doors_recently_closed = (frame.data[3] == 0x40);
    d.driver_front    = (frame.data[4] & 0x20) != 0;
    d.passenger_front = (frame.data[4] & 0x10) != 0;
    d.driver_rear     = (frame.data[4] & 0x08) != 0;
    d.passenger_rear  = (frame.data[4] & 0x04) != 0;
    d.trunk           = (frame.data[4] & 0x01) != 0;
}

void CanController::sendPacketToCan(packet_t* packet) {
    CANMessage frame;
    frame.id  = packet->id;
    frame.ext = packet->ide != 0;
    frame.rtr = packet->rtr != 0;
    frame.len = packet->dlc > 8 ? 8 : packet->dlc;
    for (uint8_t i = 0; i < frame.len; i++) {
        frame.data[i] = packet->dataArray[i];
    }
    for (int retries = 10; retries > 0; retries--) {
        if (gCan.tryToSend(frame)) return;
    }
}

//------------------------------------------------------------------------------
// Serial parser
char CanController::getNum(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

char* CanController::strToHex(char* str, byte* hexArray, byte* len) {
    byte* ptr = hexArray;
    char* idx;
    for (idx = str; *idx != SEPARATOR && *idx != TERMINATOR; ++idx, ++ptr) {
        uint8_t hi = getNum(*idx++);
        *ptr = (hi << 4) + getNum(*idx);
    }
    *len = ptr - hexArray;
    return idx;
}

void CanController::rxParse(char* buf, int len) {
    packet_t rxPacket;
    char* ptr = buf;

    byte idTempArray[8], tempLen;
    ptr = strToHex(ptr, idTempArray, &tempLen);
    rxPacket.id = 0;
    for (int i = 0; i < tempLen; i++) {
        rxPacket.id |= (long)idTempArray[i] << ((tempLen - i - 1) * 8);
    }

    ptr = strToHex(ptr + 1, &rxPacket.rtr, &tempLen);
    ptr = strToHex(ptr + 1, &rxPacket.ide, &tempLen);
    ptr = strToHex(ptr + 1, rxPacket.dataArray, &rxPacket.dlc);

#if RANDOM_CAN == 1
    printPacket(&rxPacket);
#else
    sendPacketToCan(&rxPacket);
#endif
}

void CanController::rxCallback() {
    static int rxPtr = 0;
    static char rxBuf[RXBUF_LEN];

    while (Serial.available() > 0) {
        if (rxPtr >= RXBUF_LEN) rxPtr = 0;
        char c = Serial.read();
        rxBuf[rxPtr++] = c;
        if (c == TERMINATOR) {
            rxParse(rxBuf, rxPtr);
            rxPtr = 0;
        }
    }
}

//------------------------------------------------------------------------------
// Print helpers
void CanController::printHex(long num) {
    if (num < 0x10) Serial.print("0");
    Serial.print(num, HEX);
}

void CanController::printPacket(packet_t* packet) {
    printHex(packet->id);
    Serial.print(SEPARATOR);
    printHex(packet->rtr);
    Serial.print(SEPARATOR);
    printHex(packet->ide);
    Serial.print(SEPARATOR);
    for (int i = 0; i < packet->dlc; i++) {
        printHex(packet->dataArray[i]);
    }
    Serial.print(TERMINATOR);
}

void CanController::printDoorChanges() {
    if (doors.driver_front != _prevDoors.driver_front) {
        Serial.print("Driver side front: ");
        Serial.println(doors.driver_front ? "OPEN" : "CLOSED");
        _prevDoors.driver_front = doors.driver_front;
    }
    if (doors.passenger_front != _prevDoors.passenger_front) {
        Serial.print("Passenger side front: ");
        Serial.println(doors.passenger_front ? "OPEN" : "CLOSED");
        _prevDoors.passenger_front = doors.passenger_front;
    }
    if (doors.driver_rear != _prevDoors.driver_rear) {
        Serial.print("Driver side back: ");
        Serial.println(doors.driver_rear ? "OPEN" : "CLOSED");
        _prevDoors.driver_rear = doors.driver_rear;
    }
    if (doors.passenger_rear != _prevDoors.passenger_rear) {
        Serial.print("Passenger side back: ");
        Serial.println(doors.passenger_rear ? "OPEN" : "CLOSED");
        _prevDoors.passenger_rear = doors.passenger_rear;
    }
    if (doors.trunk != _prevDoors.trunk) {
        Serial.print("Trunk: ");
        Serial.println(doors.trunk ? "OPEN" : "CLOSED");
        _prevDoors.trunk = doors.trunk;
    }
}
