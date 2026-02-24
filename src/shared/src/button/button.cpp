#include <button/button.h>
#include <Arduino.h>
#include <stdint.h>

Button::Button(uint8_t pin) : _pin(pin) {}

void Button::init() {
    pinMode(_pin, INPUT);
}

bool Button::isPressed() {
    return digitalRead(_pin) == HIGH;
}