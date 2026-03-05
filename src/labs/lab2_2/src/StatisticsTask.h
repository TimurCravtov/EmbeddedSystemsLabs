#pragma once

#include <stdint.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

namespace Statistics {
    extern uint16_t shortPressesNumber;
    extern unsigned long shortPressesTotalDuration;

    extern uint16_t longPressesNumber;
    extern unsigned long longPressesTotalDuration;

    extern SemaphoreHandle_t statsMutex;
}

namespace StatisticsTask {
    void run(void* parameters);
}