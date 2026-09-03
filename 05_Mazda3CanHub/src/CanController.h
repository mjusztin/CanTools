#pragma once
#include <Arduino.h>
#include <ACAN2515Tiny.h>
#include "DoorState.h"

#define RANDOM_CAN 0

#define MCP2515_CS     10
#define MCP2515_INT    2   // ATmega328 external interrupts are only on pins 2 and 3
#define MCP2515_QUARTZ (8UL * 1000UL * 1000UL)

typedef struct {
    long id;
    byte rtr;
    byte ide;
    byte dlc;
    byte dataArray[20];
} packet_t;

// Values match the raw field in 0x228 byte 0, bits 0-2.
enum Gear : uint8_t {
    GEAR_UNKNOWN = 0,
    GEAR_PARK    = 1,
    GEAR_REVERSE = 2,
    GEAR_NEUTRAL = 3,
    GEAR_DRIVE   = 4,
};

class CanController {
public:
    CanController();
    bool begin(long canSpeed);
    void update();

    DoorState doors;
    bool isDark = false;
    Gear gear = GEAR_UNKNOWN;

private:
    DoorState _prevDoors;
    static const char SEPARATOR  = ',';
    static const char TERMINATOR = '\n';
    static const int  RXBUF_LEN  = 100;

    static void printHex(long num);
    void printPacket(packet_t* packet);
    void sendPacketToCan(packet_t* packet);
    static char getNum(char c);
    char* strToHex(char* str, byte* hexArray, byte* len);
    void rxParse(char* buf, int len);
    void rxCallback();
    void printDoorChanges();

#if RANDOM_CAN == 1
    void CANsimulate();
#endif

    // Acceptance-filter callbacks. The library takes plain function pointers,
    // so these are static and reach the object through _instance.
    static void onLightSensorFrame(const CANMessage& frame);
    static void onDoorFrame(const CANMessage& frame);
    static void onGearFrame(const CANMessage& frame);

    static CanController* _instance;
};
