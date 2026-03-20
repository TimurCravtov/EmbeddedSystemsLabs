#ifndef GENERIC_ANALOG_SENSOR_H
#define GENERIC_ANALOG_SENSOR_H

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

#endif // GENERIC_ANALOG_SENSOR_H
