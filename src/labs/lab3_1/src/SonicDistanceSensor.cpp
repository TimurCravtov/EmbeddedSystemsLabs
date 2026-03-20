#include "SonicDistanceSensor.h"
#include <Arduino.h>

SonicDistanceSensor::SonicDistanceSensor(int trig, int echo)
    : trigPin(trig), echoPin(echo) {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

float SonicDistanceSensor::read() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 30000);
    if (duration == 0) return -1.0f;
    return (duration * 0.0343f) / 2.0f;
}
