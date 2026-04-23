#include "sensors/distance.h"
#include <Arduino_FreeRTOS.h>

volatile uint32_t echoStart = 0;
volatile uint32_t echoEnd = 0;
volatile bool echoReady = false;
static uint8_t _echoPin;
static bool _useInterrupt = false;

void echoISR() {
    if (digitalRead(_echoPin)) echoStart = micros();
    else { echoEnd = micros(); echoReady = true; }
}

DistanceSensor::DistanceSensor(uint8_t triggerPin, uint8_t echoPin) 
    : triggerPin(triggerPin), echoPin(echoPin) { }

void DistanceSensor::_init() {
    pinMode(this->triggerPin, OUTPUT);
    pinMode(this->echoPin, INPUT);
    _useInterrupt = false; // Using pulseIn directly
}

float DistanceSensor::_readRaw() {
    digitalWrite(this->triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(this->triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(this->triggerPin, LOW);

    unsigned long duration = pulseIn(this->echoPin, HIGH, 30000UL);
    if (duration == 0) return 0;
    return duration * 0.034f / 2.0f;
}