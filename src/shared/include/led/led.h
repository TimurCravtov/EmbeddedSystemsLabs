#pragma once

#include <Arduino.h>

/// @brief A simple LED control class
class Led {
public:
  explicit Led(uint8_t pin);

  void on();
  boolean isOn();
  void init();
  void off();
  void toggle();

private:
  uint8_t pin_;
};
