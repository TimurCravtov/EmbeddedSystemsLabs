#pragma once

#include <stdint.h>
#include "Task.h"

/// @brief A simple cooperative scheduler for managing tasks in an embedded system. It allows you to add tasks with specific recurrence delays and offsets, and it executes them in a loop based on their configured timing.
namespace Scheduler {
    /// @brief An array of TaskConfig with dynamic size. Used internally by the implentation
    extern TaskConfig* tasks;

    /// @brief The number of tasks currently registered in the scheduler. Used internally by the implementation.
    extern uint8_t taskCount;

    /// @brief Adds a new task to the scheduler with the specified function, recurrence delay, and offset. The task will be executed based on its timing configuration.
    void addTask(void (*taskFunction)(), uint16_t recurrenceDelay, uint16_t offset);

    /// @brief Initializes the scheduler by adding tasks to it. This function should be called in the setup() function of the main program to register all tasks before the main loop starts. Since implementation of this function is in main.cpp, it is declared as extern here.
    extern void setup();

    /// @brief This will be called in the main loop to execute the tasks based on their timing. It checks each task's recurrence control and executes the task function when the control reaches zero, then resets the control to the task's recurrence delay.
    void loop();
    
    /// @brief Registers a task with the given configuration. This function is used internally to set up the task's function pointer, recurrence delay, offset, and initial recurrence control based on the offset.
    /// @param config A pointer to the TaskConfig structure that will be filled with the task's configuration.
    /// @param taskFunction A pointer to the function that implements the task's behavior.
    /// @param recurrenceDelay The delay in ticks between each execution of the task after the initial offset.
    /// @param offset The initial delay in ticks before the task is first executed.
    void registerTask(TaskConfig *config, void (*taskFunction)(), uint16_t recurrenceDelay, uint16_t offset);
}
