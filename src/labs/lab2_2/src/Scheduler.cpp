#include "Scheduler.h"
#include "Task.h"
#include <stdio.h>
#include <stdlib.h>

#include "StatisticsTask.h"
#include "ReadingTask.h"
#include "DisplayTask.h"

namespace Scheduler {
    TaskConfig* tasks = nullptr;
    uint8_t taskCount = 0;

    void addTask(void (*taskFunction)(), uint16_t recurrenceDelay, uint16_t offset) {
        TaskConfig* temp = (TaskConfig*)realloc(tasks, (taskCount + 1) * sizeof(TaskConfig));
        if (temp != nullptr) {
            tasks = temp;
            registerTask(&tasks[taskCount], taskFunction, recurrenceDelay, offset);
            taskCount++;
        } else {
            fprintf(stderr, "Failed to allocate memory for tasks\n");
        }
    }

    void registerTask(TaskConfig *config, void (*taskFunction)(), uint16_t recurrenceDelay, uint16_t offset) {
        config->taskFunction = taskFunction;
        config->recurrenceDelay = recurrenceDelay;
        config->offset = offset;
        config->recurrenceControl = offset;
    }

    void loop() {
        for (uint8_t i = 0; i < taskCount; i++) {
            if (--tasks[i].recurrenceControl == 0) {
                tasks[i].taskFunction();
                tasks[i].recurrenceControl = tasks[i].recurrenceDelay;
            }
        }
    }
}