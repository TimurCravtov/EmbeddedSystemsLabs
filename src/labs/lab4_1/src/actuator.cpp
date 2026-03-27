#include "actuator.h"

RelayActuator::RelayActuator(uint8_t p) : pin(p), state(false) {}

void RelayActuator::begin() {
    pinMode(pin, OUTPUT);
    off();
}

void RelayActuator::on() {
    digitalWrite(pin, HIGH);
    state = true;
}

void RelayActuator::off() {
    digitalWrite(pin, LOW);
    state = false;
}

bool RelayActuator::isOn() {
    return state;
}