#pragma once

#include <Arduino.h>
#include <stdint.h>


template <typename Derived>
class Sensor {
public:
    void init() {
        static_cast<Derived*>(this)->_init();
    }

    float readRaw() {
        return static_cast<Derived*>(this)->_readRaw();
    }
};

class DistanceSensor: public Sensor<DistanceSensor> {
public:
    DistanceSensor(uint8_t triggerPin, uint8_t echoPin);
    float _readRaw();
    void _init();
private:
    uint8_t triggerPin;
    uint8_t echoPin;
};

