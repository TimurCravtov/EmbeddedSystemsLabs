#include "actuator.h"

// Helper: median of 3 values
static uint8_t median3(uint8_t a, uint8_t b, uint8_t c) {
  if (a > b) { uint8_t t = a; a = b; b = t; }
  if (b > c) { uint8_t t = b; b = c; c = t; }
  if (a > b) return a;
  return b;
}

// Helper: saturate to limits
static uint8_t saturate(uint8_t val, uint8_t min_val, uint8_t max_val) {
  if (val < min_val) return min_val;
  if (val > max_val) return max_val;
  return val;
}

// PwmLed implementation
PwmLed::PwmLed(uint8_t p) : pin(p), ema_value(0), target(0), current(0) {
  state = {0, 0, 0, 0, 0, false};
  median_buf[0] = median_buf[1] = median_buf[2] = 0;
  median_idx = 0;
}

void PwmLed::begin() {
  pinMode(pin, OUTPUT);
  analogWrite(pin, 0);
}

void PwmLed::setRaw(uint8_t value) {
  state.raw = value;
}

void PwmLed::update() {
  // 1. Saturation (0-100%)
  state.saturated = saturate(state.raw, 0, 100);
  state.limit_alert = (state.raw > 100);

  // 2. Median filter (remove impulse noise)
  median_buf[median_idx] = state.saturated;
  median_idx = (median_idx + 1) % 3;
  state.filtered = median3(median_buf[0], median_buf[1], median_buf[2]);

  // 3. Weighted average / EMA (reduce fluctuations)
  ema_value = 0.2f * state.filtered + 0.8f * ema_value;
  state.smoothed = (uint8_t)(ema_value + 0.5f);

  // 4. Ramping (soft start/stop)
  target = state.smoothed;
  if (current < target) current++;
  else if (current > target) current--;
  state.output = current;

  // Apply output
  analogWrite(pin, map(state.output, 0, 100, 0, 255));
}

// ServoMotor implementation
ServoMotor::ServoMotor(uint8_t p) : pin(p), ema_value(0), target(0), current(0) {
  state = {0, 0, 0, 0, 0, false};
  median_buf[0] = median_buf[1] = median_buf[2] = 0;
  median_idx = 0;
}

void ServoMotor::begin() {
  this->servo.attach(pin);
  this->servo.write(0);
}

void ServoMotor::setRaw(uint8_t value) {
  state.raw = value;
}

void ServoMotor::update() {
  // 1. Saturation
  state.saturated = saturate(state.raw, 0, 100);
  state.limit_alert = (state.raw > 100);

  // 2. Median filter
  median_buf[median_idx] = state.saturated;
  median_idx = (median_idx + 1) % 3;
  state.filtered = median3(median_buf[0], median_buf[1], median_buf[2]);

  // 3. Weighted average / EMA
  ema_value = 0.2f * state.filtered + 0.8f * ema_value;
  state.smoothed = (uint8_t)(ema_value + 0.5f);

  // 4. Ramping
  target = state.smoothed;
  if (current < target) current++;
  else if (current > target) current--;
  state.output = current;

  // Apply to servo (0-100% => 0-180)
  this->servo.write(map(state.output, 0, 100, 0, 180));
}