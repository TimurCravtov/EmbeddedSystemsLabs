#include "actuator.h"

ServoActuator::ServoActuator(uint8_t p) : pin(p), currentPosition(0), currentAngle(0) {}

void ServoActuator::begin() {
    servo.attach(pin);
    servo.write(0);  // Start at 0 degrees
}

void ServoActuator::setPosition(uint8_t position) {
    // Saturate to valid range
    if (position > ACTUATOR_MAX) position = ACTUATOR_MAX;
    
    currentPosition = position;
    // Map 0-100% to 0-180 degrees
    currentAngle = map(position, ACTUATOR_MIN, ACTUATOR_MAX, 0, 180);
    servo.write(currentAngle);
}

uint8_t ServoActuator::getPosition() {
    return currentPosition;
}

uint8_t ServoActuator::getAngle() {
    return currentAngle;
}
