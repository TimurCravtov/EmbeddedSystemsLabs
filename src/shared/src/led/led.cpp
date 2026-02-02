#include <led/led.h>

Led::Led(uint8_t pin) : pin_(pin) {
  pinMode(pin_, OUTPUT);
}

void Led::on() {
  digitalWrite(pin_, HIGH);
}

boolean Led::isOn() {
  return digitalRead(pin_) == HIGH;
}

void Led::off() {
  digitalWrite(pin_, LOW);
}

void Led::toggle() {
  digitalWrite(pin_, !digitalRead(pin_));
}
 