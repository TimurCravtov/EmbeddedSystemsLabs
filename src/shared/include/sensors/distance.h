#pragma once

#include <Arduino.h>
#include <stdint.h>

class DistanceSensor {
public:
    DistanceSensor(uint8_t triggerPin, uint8_t echoPin);
    virtual ~DistanceSensor() = default;
    virtual float getDistance() const = 0;
    void init() const;
private:
    uint8_t triggerPin;
    uint8_t echoPin;
};