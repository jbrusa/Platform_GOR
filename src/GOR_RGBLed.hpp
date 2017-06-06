//
// Created by Jordi Brusa on 31.05.17.
//

#ifndef PLATFORM_GOR_GOR_RGBLED_HPP
#define PLATFORM_GOR_GOR_RGBLED_HPP

#define COLOR_RED  Color(0xff, 0x00, 0x00)
#define COLOR_GREEN Color(0x00, 0xff, 0x00)
#define COLOR_ORANGE Color(0xff, 0xff, 0x00)
#define COLOR_BLUE Color(0x00, 0x00, 0xff)

extern PubSubClient mqtt;

struct Color {
    uint8_t r, g, b;

    Color() { set(0,0,0); }
    Color(uint8_t r, uint8_t g ,uint8_t b) { set(r,g,b); }

    void set(uint8_t r, uint8_t g ,uint8_t b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

class GOR_RGBLed {
private:
    uint8_t pin_r, pin_g, pin_b;

public:

    void begin(uint8_t pin_r, uint8_t pin_g, uint8_t pin_b) {
        this->pin_r = pin_r;
        this->pin_g = pin_g;
        this->pin_b = pin_b;
        pinMode(pin_r, OUTPUT);
        pinMode(pin_g, OUTPUT);
        pinMode(pin_b, OUTPUT);
    }

    void setColor(Color c) {
        digitalWrite(pin_r, (c.r) ? HIGH : LOW);
        digitalWrite(pin_g, (c.g) ? HIGH : LOW);
        digitalWrite(pin_b, (c.b) ? HIGH : LOW);
    }
};

#endif //PLATFORM_GOR_GOR_RGBLED_HPP
