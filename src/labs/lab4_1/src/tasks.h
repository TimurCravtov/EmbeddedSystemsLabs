#ifndef TASKS_H
#define TASKS_H

#include "actuator.h"
#include <Arduino_FreeRTOS.h>

// Shared state variables (accessed by tasks)
extern volatile bool rawActuatorState;
extern volatile bool stableActuatorState;

// Task function declarations
void TaskRead(void* pvParameters);
void TaskActuatorConditioning(void* pvParameters);
void TaskWrite(void* pvParameters);

#endif // TASKS_H