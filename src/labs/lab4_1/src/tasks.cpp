#include "tasks.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <serialio/serialio.h>
#include <lcd/LcdStdioManager.h>

// Shared state variables
volatile bool rawActuatorState = false;
volatile bool stableActuatorState = false;

// Reads keypad input via stdin
void TaskRead(void* pvParameters) {
    while(true) {
        char c = 0;
        int result = scanf("%c", &c);

        if (result > 0 && c != '\0' && c != '\n' && c != '\r') {
            if (c == '1') {
                rawActuatorState = true;
            } else if (c == '0') {
                rawActuatorState = false;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(60));
    }
}

// Debounces raw state
void TaskActuatorConditioning(void* pvParameters) {
    const uint8_t debounce = 3;
    uint8_t stableCount = 0;
    bool lastRaw = rawActuatorState;

    while(true) {
        bool currentRaw = rawActuatorState;

        if (currentRaw == lastRaw) {
            if (stableCount < debounce) {
                stableCount++;
            }
            if (stableCount >= debounce && stableActuatorState != currentRaw) {
                stableActuatorState = currentRaw;
            }
        } else {
            lastRaw = currentRaw;
            stableCount = 1;
        }
        vTaskDelay(pdMS_TO_TICKS(60));
    }
}

// Updates relay and prints to lcd via stdout
void TaskWrite(void* pvParameters) {
    RelayActuator* relay = static_cast<RelayActuator*>(pvParameters);

    while(true) {
        if (stableActuatorState) {
            if (!relay->isOn()) relay->on();
            printf("\r                    \r"); // clears line
            printf("Actuator is ON\r");
        } else {
            if (relay->isOn()) relay->off();
            printf("\r                    \r"); // clears line
            printf("Actuator is OFF\r");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}