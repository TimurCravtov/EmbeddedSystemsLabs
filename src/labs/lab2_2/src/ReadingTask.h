#pragma once

#include <stdint.h>

namespace ReadingTask {
    void run(void* parameters);
    extern uint16_t recurrenceDelay;
    extern uint16_t offset;
    extern uint32_t lastDuration;
}

