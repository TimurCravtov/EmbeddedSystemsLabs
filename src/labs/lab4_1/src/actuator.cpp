#include "actuator.h"

PwmActuator::PwmActuator(uint8_t p) : pin(p), currentSpeed(0), currentPwm(0) {}

void PwmActuator::begin() {
    pinMode(pin, OUTPUT);
    analogWrite(pin, 0);
}

void PwmActuator::setSpeed(uint8_t speed) {
    // Saturate to valid range
    if (speed > ACTUATOR_MAX) speed = ACTUATOR_MAX;
    
    currentSpeed = speed;
    currentPwm = map(speed, ACTUATOR_MIN, ACTUATOR_MAX, 0, PWM_MAX);
    analogWrite(pin, currentPwm);
}

uint8_t PwmActuator::getSpeed() {
    return currentSpeed;
}

uint8_t PwmActuator::getPwm() {
    return currentPwm;
}
