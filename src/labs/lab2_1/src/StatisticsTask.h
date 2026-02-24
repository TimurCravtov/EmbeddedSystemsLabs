#pragma once

#include <stdint.h>

namespace Statistics {
    extern uint16_t shortPressesNumber;
    extern uint16_t longPressesNumber;
    extern uint16_t totalTimePressed;
}

namespace StatisticsTask {
    void run();
    extern uint16_t recurrenceDelay;
    extern uint16_t offset;
}
