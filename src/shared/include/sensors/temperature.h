#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "sensors/distance.h" // Sensor<> base

class TemperatureSensor : public Sensor<TemperatureSensor> {
public:
    TemperatureSensor(uint8_t analogPin);
    float _readRaw();
    void  _init();
private:
    uint8_t analogPin;
};
