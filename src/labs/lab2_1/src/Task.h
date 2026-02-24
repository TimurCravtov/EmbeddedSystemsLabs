#pragma once

#include <stdint.h>

struct TaskConfig {
  uint16_t recurrenceControl;
  void (*taskFunction)();
  uint16_t recurrenceDelay;
  uint16_t offset;
};
