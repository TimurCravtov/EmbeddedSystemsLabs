#pragma once

#include <Arduino.h>

// Configuration constants
constexpr uint8_t ACTUATOR_MIN = 0;
constexpr uint8_t ACTUATOR_MAX = 100;
constexpr uint16_t PWM_MAX = 255;

// Abstract interface for analog actuators
class AnalogActuator {
  public:
    virtual void setSpeed(uint8_t speed) = 0;
    virtual uint8_t getSpeed() = 0;
    virtual void begin() = 0;
    virtual ~AnalogActuator() {}
};

// Concrete implementation for PWM-based actuator control
class PwmActuator : public AnalogActuator {
  private:
    uint8_t pin;
    uint8_t currentSpeed;  // 0-100%
    uint8_t currentPwm;    // 0-255

  public:
    PwmActuator(uint8_t p);
    void begin() override;
    void setSpeed(uint8_t speed) override;
    uint8_t getSpeed() override;
    uint8_t getPwm();
};