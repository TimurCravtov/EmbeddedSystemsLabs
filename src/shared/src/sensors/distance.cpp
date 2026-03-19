#include "sensors/distance.h"

DistanceSensor::DistanceSensor(uint8_t triggerPin, uint8_t echoPin) : triggerPin(triggerPin), echoPin(echoPin) { }

void DistanceSensor::_init() {
    pinMode(this->triggerPin, OUTPUT);
    pinMode(this->echoPin, INPUT);
}



float DistanceSensor::_readRaw() {
    digitalWrite(this->triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(this->triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(this->triggerPin, LOW);

    long duration = pulseIn(this->echoPin, HIGH, 30000);
    float distance = duration * 0.034 / 2; // Speed of sound is 343 m/s, which is 0.034 cm/us
    return distance;
}