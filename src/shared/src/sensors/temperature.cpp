#include "sensors/temperature.h"

TemperatureSensor::TemperatureSensor(uint8_t analogPin) : analogPin(analogPin) { }

void TemperatureSensor::_init() {
    pinMode(this->analogPin, INPUT);
}

float TemperatureSensor::_readRaw() {
    return (float) analogRead(this->analogPin);
}
