#pragma once
#include <Arduino.h>
#include <CAN.h>
#include "DoorState.h"

#define RANDOM_CAN 0

typedef struct {
    long id;
    byte rtr;
    byte ide;
    byte dlc;
    byte dataArray[20];
} packet_t;

class CanController {
public:
    CanController();
    bool begin(long canSpeed);
    void update();

    DoorState doors;

private:
    DoorState _prevDoors;
    static const char SEPARATOR  = ',';
    static const char TERMINATOR = '\n';
    static const int  RXBUF_LEN  = 100;

    static void printHex(long num);
    void printPacket(packet_t* packet);
    void sendPacketToCan(packet_t* packet);
    void filterMessage(uint32_t id, uint8_t* data);
    static char getNum(char c);
    char* strToHex(char* str, byte* hexArray, byte* len);
    void rxParse(char* buf, int len);
    void rxCallback();
    void printDoorChanges();

#if RANDOM_CAN == 1
    void CANsimulate();
#endif

    static void onCANReceive(int packetSize);
    static CanController* _instance;
};
