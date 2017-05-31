//
// Created by Jordi Brusa on 31.05.17.
//

#ifndef PLATFORM_GOR_GOR_RGBLED_HPP
#define PLATFORM_GOR_GOR_RGBLED_HPP

#define RGBLED_R_PIN    23
#define RGBLED_G_PIN    24
#define RGBLED_B_PIN    25

extern PubSubClient mqtt;

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    Color() {
        set(0,0,0);
    }

    Color(uint8_t r, uint8_t g ,uint8_t b) {
        set(r,g,b);
    }

    void set(uint8_t r, uint8_t g ,uint8_t b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

class GOR_RGBLed {
public:

    void begin() {
        pinMode(RGBLED_R_PIN, OUTPUT);
        pinMode(RGBLED_G_PIN, OUTPUT);
        pinMode(RGBLED_B_PIN, OUTPUT);
    }

    void setColor(Color c) {
        digitalWrite(RGBLED_R_PIN, (c.r) ? HIGH : LOW);
        digitalWrite(RGBLED_G_PIN, (c.g) ? HIGH : LOW);
        digitalWrite(RGBLED_B_PIN, (c.b) ? HIGH : LOW);
    }
};

#endif //PLATFORM_GOR_GOR_RGBLED_HPP
