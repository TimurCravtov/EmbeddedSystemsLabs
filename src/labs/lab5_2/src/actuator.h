#pragma once

#include <Arduino.h>
#include <Servo.h>

// Configuration constants
constexpr uint8_t ACTUATOR_MIN = 0;
constexpr uint8_t ACTUATOR_MAX = 100;

// Abstract interface for analog actuators
class AnalogActuator {
  public:
    virtual void setPosition(uint8_t position) = 0;
    virtual uint8_t getPosition() = 0;
    virtual void begin() = 0;
    virtual ~AnalogActuator() {}
};

// Concrete implementation for Servo-based actuator control
// Maps 0-100% to servo angle (0-180 degrees)
class ServoActuator : public AnalogActuator {
  private:
    uint8_t pin;
    Servo servo;
    uint8_t currentPosition;  // 0-100%
    uint8_t currentAngle;     // 0-180 degrees

  public:
    ServoActuator(uint8_t p);
    void begin() override;
    void setPosition(uint8_t position) override;
    uint8_t getPosition() override;
    uint8_t getAngle();
};
