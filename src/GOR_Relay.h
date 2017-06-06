
#ifndef PLATFORM_GOR_GOR_RELAY_HPP
#define PLATFORM_GOR_GOR_RELAY_HPP

#include "Arduino.h"

class GOR_Relay {
private:
    uint8_t pin;
    uint8_t state;
public:
    GOR_Relay() { }

    void begin(uint8_t pin);
    void setState(uint8_t newstate);

    uint8_t getState() { return state; }
};

#endif //PLATFORM_GOR_GOR_RELAY_HPP
