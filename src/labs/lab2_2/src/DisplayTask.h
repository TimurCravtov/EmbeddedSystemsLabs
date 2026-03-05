#pragma once

#include <stdint.h>

namespace DisplayTask {
    void run(void* parameters);
    extern uint16_t recurrenceDelay;
    extern uint16_t offset;
}
