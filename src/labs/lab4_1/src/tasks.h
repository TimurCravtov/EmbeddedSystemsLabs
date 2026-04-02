#ifndef TASKS_H
#define TASKS_H

#include "actuator.h"
#include <Arduino_FreeRTOS.h>

// Task periods (ms)
constexpr uint32_t TASK_READ_PERIOD = 50;       // Keypad reading period
constexpr uint32_t TASK_FILTER_PERIOD = 100;    // Filtering period
constexpr uint32_t TASK_DISPLAY_PERIOD = 500;   // LCD display period

// Shared state variables (accessed by tasks)
extern volatile uint8_t rawSpeed;           // Raw input speed (0-100%)
extern volatile uint8_t filteredSpeed;      // Filtered speed (0-100%)
extern volatile uint8_t outputSpeed;        // Output speed with ramping (0-100%)
extern volatile bool commandReady;          // Flag for new command
extern volatile bool limitReached;          // Alert: limit reached
extern volatile bool overloadDetected;      // Alert: overload

// System state
struct SystemState {
    uint8_t raw;
    uint8_t filtered;
    uint8_t output;
    bool limitAlert;
    bool overloadAlert;
};

extern volatile SystemState sysState;

// Task function declarations
void TaskRead(void* pvParameters);      // Read commands from keypad
void TaskFilter(void* pvParameters);    // Filter and condition signal
void TaskDisplay(void* pvParameters);   // Display on LCD

#endif // TASKS_H
