#pragma once

#include <stdint.h>
#include "Task.h"

namespace Scheduler {
    extern TaskConfig* tasks;
    extern uint8_t taskCount;
    void addTask(void (*taskFunction)(), uint16_t recurrenceDelay, uint16_t offset);
    extern void setup();
    void loop();
    void registerTask(TaskConfig *config, void (*taskFunction)(), uint16_t recurrenceDelay, uint16_t offset);
}

