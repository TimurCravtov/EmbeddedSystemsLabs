#include "tasks.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <serialio/serialio.h>
#include <lcd/LcdStdioManager.h>

// Shared state
volatile uint8_t rawDutyCycle = 30;
volatile uint8_t stableDutyCycle = 30;
volatile bool rampMode = false;

// Global actuator pointers
PwmLed* g_pwmLed = nullptr;
ServoMotor* g_servo = nullptr;

// TaskRead: keypad input
void TaskRead(void *pvParameters) {
  while (true) {
    char c = 0;
    int result = scanf("%c", &c);

    if (result > 0 && c != '\0' && c != '\n' && c != '\r') {
      if (c >= '0' && c <= '9') {
        rawDutyCycle = (c - '0') * 10;
      } else if (c == 'A' || c == 'a') {
        rawDutyCycle = 100;
      } else if (c == 'B' || c == 'b') {
        if (rawDutyCycle <= 90) rawDutyCycle += 10;
      } else if (c == 'C' || c == 'c') {
        if (rawDutyCycle >= 10) rawDutyCycle -= 10;
      } else if (c == 'D' || c == 'd') {
        rawDutyCycle = 50;
      } else if (c == '#') {
        rawDutyCycle = 0;
      } else if (c == '*') {
        rampMode = !rampMode;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(60));
  }
}

// TaskConditioning: update actuators
void TaskConditioning(void *pvParameters) {
  while (true) {
    if (g_pwmLed) g_pwmLed->setRaw(rawDutyCycle);
    if (g_servo) g_servo->setRaw(rawDutyCycle);

    // Update conditioning pipeline
    if (g_pwmLed) g_pwmLed->update();
    if (g_servo) g_servo->update();

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// TaskRamp: handle ramp mode display
void TaskRamp(void *pvParameters) {
  while (true) {
    fprintf(stderr, "Ramp mode: %s\n", rampMode ? "ON" : "OFF");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// TaskWrite: display output
void TaskWrite(void *pvParameters) {
  while (true) {
    ActuatorState ledState = g_pwmLed ? g_pwmLed->getState()
                                      : ActuatorState{0, 0, 0, 0, 0, false};
    ActuatorState servoState = g_servo ? g_servo->getState()
                                       : ActuatorState{0, 0, 0, 0, 0, false};

    // Display on LCD (line 1: LED and Servo, line 2: Ramp mode)
    printf("\rLED:%3d%% Sv:%3d%%\n", ledState.output, servoState.output);
    printf("Ramp: %s        \r", rampMode ? "ON " : "OFF");

    // Debug output to serial
    if (ledState.limit_alert || servoState.limit_alert) {
      fprintf(stderr, "WARNING: Value exceeded 100%%!\n");
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
