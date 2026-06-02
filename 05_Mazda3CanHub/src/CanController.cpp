#include "CanController.h"

CanController* CanController::_instance = nullptr;

CanController::CanController() {}

bool CanController::begin(long canSpeed) {
    _instance = this;
#if RANDOM_CAN == 0
    if (!CAN.begin(canSpeed)) return false;
    CAN.onReceive(onCANReceive);
#endif
    return true;
}

void CanController::update() {
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
void CanController::onCANReceive(int packetSize) {
    if (!_instance) return;
    packet_t rxPacket;
    rxPacket.id  = CAN.packetId();
    rxPacket.rtr = CAN.packetRtr() ? 1 : 0;
    rxPacket.ide = CAN.packetExtended() ? 1 : 0;
    rxPacket.dlc = CAN.packetDlc();
    byte i = 0;
    while (CAN.available()) {
        rxPacket.dataArray[i++] = CAN.read();
        if (i >= sizeof(rxPacket.dataArray) / sizeof(rxPacket.dataArray[0])) break;
    }
    _instance->filterMessage(rxPacket.id, rxPacket.dataArray);
    //_instance->printPacket(&rxPacket);
}

void CanController::sendPacketToCan(packet_t* packet) {
    for (int retries = 10; retries > 0; retries--) {
        bool rtr = packet->rtr ? true : false;
        if (packet->ide) {
            CAN.beginExtendedPacket(packet->id, packet->dlc, rtr);
        } else {
            CAN.beginPacket(packet->id, packet->dlc, rtr);
        }
        CAN.write(packet->dataArray, packet->dlc);
        if (CAN.endPacket()) break;
        else if (retries <= 1) return;
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
        *ptr = (getNum(*idx++) << 4) + getNum(*idx);
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
// CAN message decoder
void CanController::filterMessage(uint32_t id, uint8_t* data) {
    switch (id) {
        case 0x43E:
            doors.any_door_open = (data[3] == 0x60);
            doors.doors_recently_closed = (data[3] == 0x40);
            doors.driver_front    = (data[4] & 0x20) != 0;
            doors.passenger_front = (data[4] & 0x10) != 0;
            doors.driver_rear     = (data[4] & 0x08) != 0;
            doors.passenger_rear  = (data[4] & 0x04) != 0;
            doors.trunk           = (data[4] & 0x01) != 0;
            break;
        default:
            break;
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
