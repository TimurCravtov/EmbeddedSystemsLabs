#pragma once

#include <Arduino.h>


class Button {
public:
    explicit Button(uint8_t pin);
    void init();
    bool isPressed();

private:
    uint8_t _pin;
};
