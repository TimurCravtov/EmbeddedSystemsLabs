#ifndef TASKS_H
#define TASKS_H
#include "actuator.h"
#include <Arduino_FreeRTOS.h>
#include <sensors/distance.h>

// Task periods (ms)
constexpr uint32_t TASK_READ_PERIOD    = 40;
constexpr uint32_t TASK_CONTROL_PERIOD = 80;
constexpr uint32_t TASK_DISPLAY_PERIOD = 250;
constexpr uint32_t TASK_PLOT_PERIOD    = 80;

// Distance limits (cm)
constexpr uint16_t MIN_DISTANCE_DEFAULT_CM = 15;
constexpr uint16_t MIN_DISTANCE_MIN_CM     = 2;
constexpr uint16_t MIN_DISTANCE_MAX_CM     = 400;

// Servo positions (%)
constexpr uint8_t SERVO_POS_MIN     = 0;
constexpr uint8_t SERVO_POS_MAX     = 40;
constexpr uint8_t SERVO_POS_CLOSED  = SERVO_POS_MIN;
constexpr uint8_t SERVO_POS_OPEN    = SERVO_POS_MAX;
constexpr uint8_t SERVO_POS_NEUTRAL = (SERVO_POS_MIN + SERVO_POS_MAX) / 2;

// Servo angle helpers (0-180 degrees)
constexpr uint8_t SERVO_ANGLE_MIN     = (uint8_t)((SERVO_POS_MIN     * 180u) / ACTUATOR_MAX);
constexpr uint8_t SERVO_ANGLE_MAX     = (uint8_t)((SERVO_POS_MAX     * 180u) / ACTUATOR_MAX);
constexpr uint8_t SERVO_ANGLE_NEUTRAL = (uint8_t)((SERVO_POS_NEUTRAL * 180u) / ACTUATOR_MAX);
constexpr uint8_t SERVO_ANGLE_DEADBAND = 5;

// Keep a small hysteresis to avoid gate chatter near threshold
constexpr uint8_t GATE_HYSTERESIS_CM = 2;

// Serial Plotter output toggle (prints only plot lines when true)
constexpr bool ENABLE_PLOTTER = true;

// PID tuning (start values; adjust on hardware)
constexpr float PID_KP           = 1.2f;
constexpr float PID_KI           = 0.02f;
constexpr float PID_KD           = 0.8f;
constexpr float PID_INTEGRAL_MIN = -120.0f;
constexpr float PID_INTEGRAL_MAX =  120.0f;
constexpr bool  PID_INVERT_OUTPUT = false;

// Shared state variables (accessed by tasks)
extern volatile uint16_t measuredDistanceCm;
extern volatile uint16_t minDistanceCm;
extern volatile uint8_t  servoAngle;
extern volatile int16_t  pidErrorCm;
extern volatile uint8_t  pidOutputPos;
extern volatile bool     gateOpen;
extern volatile bool     sensorValid;
extern volatile char     lastKeyPressed;

// System state
struct SystemState {
    uint16_t distanceCm;
    uint16_t thresholdCm;
    uint8_t  angle;
    int16_t  errorCm;
    uint8_t  outputPos;
    bool     isOpen;
    bool     isSensorValid;
    char     key;
};
extern volatile SystemState sysState;

struct ControlContext {
    ServoActuator*  actuator;
    DistanceSensor* sensor;
};

// Task function declarations
void TaskRead(void* pvParameters);
void TaskFilter(void* pvParameters);
void TaskDisplay(void* pvParameters);

#endif // TASKS_H