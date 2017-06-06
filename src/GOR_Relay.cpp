//
// Created by Jordi Brusa on 06.06.17.
//

#include "GOR_Relay.h"

void GOR_Relay::begin(uint8_t pin) {
    this->pin = pin;
    pinMode(pin, OUTPUT);
    setState(LOW);
}

void GOR_Relay::setState(uint8_t newstate) {
    digitalWrite(pin, newstate);
    state = newstate;
}

