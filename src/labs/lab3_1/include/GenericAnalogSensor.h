#pragma once

#include "ISensor.h"

typedef float (*Transformation)(float);
float defaultTransform(float x);

class GenericAnalogSensor : public ISensor {
private:
    int pin;
    Transformation transform;
public:
    GenericAnalogSensor(int p, Transformation t = defaultTransform);
    float read() override;
};
