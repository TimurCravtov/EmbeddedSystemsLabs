#ifndef TASKS_H
#define TASKS_H

#include "actuator.h"
#include <Arduino_FreeRTOS.h>
#include <sensors/distance.h>

// Task periods (ms)
constexpr uint32_t TASK_READ_PERIOD = 40;
constexpr uint32_t TASK_CONTROL_PERIOD = 80;
constexpr uint32_t TASK_DISPLAY_PERIOD = 250;

// Distance limits (cm)
constexpr uint16_t MIN_DISTANCE_DEFAULT_CM = 20;
constexpr uint16_t MIN_DISTANCE_MIN_CM = 2;
constexpr uint16_t MIN_DISTANCE_MAX_CM = 400;

// Servo positions (%)
constexpr uint8_t SERVO_POS_CLOSED = 0;
constexpr uint8_t SERVO_POS_OPEN = 100;

// Keep a small hysteresis to avoid gate chatter near threshold
constexpr uint8_t GATE_HYSTERESIS_CM = 2;

// Shared state variables (accessed by tasks)
extern volatile uint16_t measuredDistanceCm;
extern volatile uint16_t minDistanceCm;
extern volatile uint8_t servoAngle;
extern volatile bool gateOpen;
extern volatile bool sensorValid;
extern volatile char lastKeyPressed;

// System state
struct SystemState {
    uint16_t distanceCm;
    uint16_t thresholdCm;
    uint8_t angle;
    bool isOpen;
    bool isSensorValid;
    char key;
};

extern volatile SystemState sysState;

struct ControlContext {
    ServoActuator* actuator;
    DistanceSensor* sensor;
};

// Task function declarations
void TaskRead(void* pvParameters);
void TaskFilter(void* pvParameters);
void TaskDisplay(void* pvParameters);

#endif // TASKS_H
