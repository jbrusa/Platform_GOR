//
// Created by Jordi Brusa on 28.05.17.
//

#ifndef PLATFORM_GOR_GOR_PLATFORM_HPP
#define PLATFORM_GOR_GOR_PLATFORM_HPP

#include "GOR_Config.hpp"

extern void GOR_Setup();
extern void GOR_Loop();

class GOR_Platform {

};

GOR_Platform GOR_Platform;

void setup() {
    GOR_Setup();
}

void loop() {
    GOR_Loop();
}

#endif //PLATFORM_GOR_GOR_PLATFORM_HPP
