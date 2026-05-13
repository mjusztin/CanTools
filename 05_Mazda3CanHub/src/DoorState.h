#pragma once

struct DoorState {
    bool any_door_open = false;
    bool doors_recently_closed = false;
    bool driver_front    = false;
    bool passenger_front = false;
    bool driver_rear     = false;
    bool passenger_rear  = false;
    bool trunk           = false;

};
