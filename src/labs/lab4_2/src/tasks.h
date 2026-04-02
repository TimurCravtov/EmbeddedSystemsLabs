#ifndef TASKS_H
#define TASKS_H

#include "actuator.h"
#include <Arduino_FreeRTOS.h>

// Shared state variables (accessed by tasks)
extern volatile uint8_t rawDutyCycle;
extern volatile uint8_t stableDutyCycle;
extern volatile bool rampMode;

// Global actuator pointers (set in main.cpp)
extern PwmLed* g_pwmLed;
extern ServoMotor* g_servo;

// Task function declarations
void TaskRead(void *pvParameters);
void TaskConditioning(void *pvParameters);
void TaskWrite(void *pvParameters);
void TaskRamp(void *pvParameters);

#endif // TASKS_H