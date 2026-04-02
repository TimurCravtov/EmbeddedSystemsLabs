#pragma once

#include <Arduino.h>
#include <Servo.h>

// Actuator state structure
typedef struct {
  uint8_t raw;          // Raw input (0-100%)
  uint8_t saturated;    // After saturation limits
  uint8_t filtered;     // After median filter
  uint8_t smoothed;     // After weighted average
  uint8_t output;       // Final output (after ramping)
  bool limit_alert;     // Limit reached alert
} ActuatorState;

// Actuator interface
class AnalogActuator {
  public:
    virtual void setRaw(uint8_t value) = 0;
    virtual void update() = 0;  // Process conditioning pipeline
    virtual uint8_t getSpeed() = 0;
    virtual ActuatorState getState() = 0;
    virtual void begin() = 0;
    virtual ~AnalogActuator() {}
};

// PWM LED actuator
class PwmLed : public AnalogActuator {
  private:
    uint8_t pin;
    ActuatorState state;
    uint8_t median_buf[3];
    uint8_t median_idx;
    float ema_value;
    uint8_t target;
    uint8_t current;

  public:
    PwmLed(uint8_t p);
    void begin() override;
    void setRaw(uint8_t value) override;
    void update() override;
    uint8_t getSpeed() override { return state.output; }
    ActuatorState getState() override { return state; }
};

// Servo actuator (0-100% => 0-180)
class ServoMotor : public AnalogActuator {
  private:
    uint8_t pin;
    ActuatorState state;
    uint8_t median_buf[3];
    uint8_t median_idx;
    float ema_value;
    uint8_t target;
    uint8_t current;
    Servo servo;

  public:
    ServoMotor(uint8_t p);
    void begin() override;
    void setRaw(uint8_t value) override;
    void update() override;
    uint8_t getSpeed() override { return state.output; }
    ActuatorState getState() override { return state; }
};