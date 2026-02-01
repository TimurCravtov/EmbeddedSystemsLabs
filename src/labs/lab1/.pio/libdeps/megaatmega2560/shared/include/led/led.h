#pragma once

#include <Arduino.h>

class Led {
public:
  explicit Led(uint8_t pin);

  void on();
  boolean isOn();
  void off();
  void toggle();

private:
  uint8_t pin_;
};
