#include "Scheduler.h"
#include "Task.h"
#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h> // Required for noInterrupts() and interrupts()

namespace Scheduler {
    TaskConfig* tasks = nullptr;
    uint8_t taskCount = 0;

    // add the tasks to the array of tasks, and set the config
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

    // sets the config for the task
    void registerTask(TaskConfig *config, void (*taskFunction)(), uint16_t recurrenceDelay, uint16_t offset) {
        config->taskFunction = taskFunction;
        config->recurrenceDelay = recurrenceDelay;
        config->offset = offset;
        config->recurrenceControl = offset;
        config->runMe = 0; 
    }

    // decrements the recurrence control for each task, and if it reaches zero, it sets runMe to 1 and resets the control to the recurrence delay
    void update() {
        for (uint8_t i = 0; i < taskCount; i++) {
            if (tasks[i].recurrenceControl == 0) {

                tasks[i].runMe++; 
                
                tasks[i].recurrenceControl = tasks[i].recurrenceDelay - 1;
            } else {
                tasks[i].recurrenceControl--;
            }
        }
    }

    // the actual loop that runs the tasks, it checks if runMe is greater than 0, if it is, it runs the task function and decrements runMe
    void loop() {
        for (uint8_t i = 0; i < taskCount; i++) {

            if (tasks[i].runMe > 0) {
                
                tasks[i].taskFunction(); 

                noInterrupts();
                tasks[i].runMe--;
                interrupts();
            }
        }
    }
}