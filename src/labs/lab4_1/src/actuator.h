#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <Arduino.h>

// Abstract interface for binary actuators
class BinaryActuator {
  public:
    virtual void on() = 0;
    virtual void off() = 0;
    virtual bool isOn() = 0;
    virtual ~BinaryActuator() {}
};

// Concrete implementation for relay control
class RelayActuator : public BinaryActuator {
  private:
    uint8_t pin;
    bool state;

  public:
    RelayActuator(uint8_t p);
    void begin();
    void on() override;
    void off() override;
    bool isOn() override;
};

#endif // ACTUATOR_H