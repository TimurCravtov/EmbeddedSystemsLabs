#include "GenericAnalogSensor.h"
#include <Arduino.h>

float defaultTransform(float x) { return x; }

GenericAnalogSensor::GenericAnalogSensor(int p, Transformation t)
    : pin(p), transform(t) {
    pinMode(pin, INPUT);
}

float GenericAnalogSensor::read() {
    int raw = analogRead(pin);
    float normalized = raw / 1023.0f;
    return transform(normalized);
}
