#pragma once

#include <stdint.h>

namespace Statistics {
    extern uint16_t shortPressesNumber;
    extern unsigned long shortPressesTotalDuration;

    extern uint16_t longPressesNumber;
    extern unsigned long longPressesTotalDuration;

    extern unsigned long lastHandledUpdateTime;
}

namespace StatisticsTask {
    void run();
    extern uint16_t recurrenceDelay;
    extern uint16_t offset;
}

namespace StatisticsLedTask {
    void run();
    void triggerBlinks(bool isLong);
    extern uint16_t recurrenceDelay;
    extern uint16_t offset;
}