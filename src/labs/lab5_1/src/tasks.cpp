#include "tasks.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

// PROGMEM strings reduce SRAM usage on AVR.
// Each line is padded to 16 chars to fully overwrite LCD leftovers.
static const char STR_LINE0[] PROGMEM = "\rD:%3u T:%3u %c   \n";
static const char STR_LINE0_INVALID[] PROGMEM = "\rD:--- T:%3u %c   \n";
static const char STR_LINE1[] PROGMEM = "\rK:%c In:%-3s      ";

// Shared state variables
volatile uint16_t measuredDistanceCm = 0;
volatile uint16_t minDistanceCm = MIN_DISTANCE_DEFAULT_CM;
volatile uint8_t servoAngle = 0;
volatile bool gateOpen = false;
volatile bool sensorValid = false;
volatile char lastKeyPressed = '-';
volatile SystemState sysState = {0, MIN_DISTANCE_DEFAULT_CM, 0, false, false, '-'};

// Input buffer for keypad threshold entry (max 3 digits: 400)
static char inputBuffer[4] = {0};
static uint8_t inputIndex = 0;

static inline void clearInputBuffer() {
    inputIndex = 0;
    inputBuffer[0] = '\0';
}

static uint16_t parseInputBuffer() {
    uint16_t value = 0;
    for (uint8_t i = 0; i < inputIndex; i++) {
        value = (uint16_t)(value * 10u + (uint16_t)(inputBuffer[i] - '0'));
    }
    return value;
}

static inline uint16_t clampThreshold(uint16_t value) {
    if (value < MIN_DISTANCE_MIN_CM) {
        return MIN_DISTANCE_MIN_CM;
    }
    if (value > MIN_DISTANCE_MAX_CM) {
        return MIN_DISTANCE_MAX_CM;
    }
    return value;
}

void TaskRead(void* pvParameters) {
    (void)pvParameters;

    while (true) {
        char c = 0;
        if (scanf("%c", &c) > 0 && c != '\0' && c != '\n' && c != '\r') {
            lastKeyPressed = c;
            sysState.key = c;

            if (c >= '0' && c <= '9') {
                if (inputIndex < 3) {
                    inputBuffer[inputIndex++] = c;
                    inputBuffer[inputIndex] = '\0';
                }
            } else if (c == '#' || c == 'A') {
                if (inputIndex > 0) {
                    minDistanceCm = clampThreshold(parseInputBuffer());
                    sysState.thresholdCm = minDistanceCm;
                    clearInputBuffer();
                }
            } else if (c == '*' || c == 'B') {
                clearInputBuffer();
            } else if (c == 'C') {
                minDistanceCm = MIN_DISTANCE_DEFAULT_CM;
                sysState.thresholdCm = minDistanceCm;
                clearInputBuffer();
            } else if (c == 'D') {
                // D is a quick +5 cm step.
                minDistanceCm = clampThreshold((uint16_t)(minDistanceCm + 5u));
                sysState.thresholdCm = minDistanceCm;
                clearInputBuffer();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(TASK_READ_PERIOD));
    }
}

void TaskFilter(void* pvParameters) {
    ControlContext* ctx = static_cast<ControlContext*>(pvParameters);
    ServoActuator* actuator = ctx->actuator;
    DistanceSensor* sensor = ctx->sensor;

    actuator->setPosition(SERVO_POS_OPEN);
    vTaskDelay(pdMS_TO_TICKS(500));
    actuator->setPosition(SERVO_POS_CLOSED);
    vTaskDelay(pdMS_TO_TICKS(300));
    gateOpen = false;
    servoAngle = actuator->getAngle();

    while (true) {
        const float rawDistance = sensor->readRaw();
        const bool valid = (rawDistance > 0.5f) && (rawDistance <= (float)MIN_DISTANCE_MAX_CM);
        const uint16_t distanceCm = valid ? (uint16_t)(rawDistance + 0.5f) : 0u;

        bool nextGateState = gateOpen;
        if (valid) {
            const uint16_t threshold = minDistanceCm;
            if (!gateOpen && distanceCm < threshold) {
                nextGateState = true;
            } else if (gateOpen && distanceCm > (uint16_t)(threshold + GATE_HYSTERESIS_CM)) {
                nextGateState = false;
            }
        } else {
            nextGateState = false;
        }

        if (nextGateState != gateOpen) {
            actuator->setPosition(nextGateState ? SERVO_POS_OPEN : SERVO_POS_CLOSED);
            gateOpen = nextGateState;
        }

        measuredDistanceCm = distanceCm;
        sensorValid = valid;
        servoAngle = actuator->getAngle();

        sysState.distanceCm = measuredDistanceCm;
        sysState.thresholdCm = minDistanceCm;
        sysState.angle = servoAngle;
        sysState.isOpen = gateOpen;
        sysState.isSensorValid = sensorValid;

        vTaskDelay(pdMS_TO_TICKS(TASK_CONTROL_PERIOD));
    }
}

void TaskDisplay(void* pvParameters) {
    (void)pvParameters;

    while (true) {
        uint16_t distanceCmLocal;
        uint16_t thresholdCmLocal;
        bool gateOpenLocal;
        bool sensorValidLocal;
        char keyLocal;
        uint8_t inputLenLocal;
        char input0;
        char input1;
        char input2;

        taskENTER_CRITICAL();
        distanceCmLocal = sysState.distanceCm;
        thresholdCmLocal = sysState.thresholdCm;
        gateOpenLocal = sysState.isOpen;
        sensorValidLocal = sysState.isSensorValid;
        keyLocal = sysState.key;
        inputLenLocal = inputIndex;
        input0 = inputBuffer[0];
        input1 = inputBuffer[1];
        input2 = inputBuffer[2];
        taskEXIT_CRITICAL();

        if (keyLocal < 32 || keyLocal > 126) {
            keyLocal = '-';
        }

        char localInput[4];
        localInput[0] = input0;
        localInput[1] = input1;
        localInput[2] = input2;
        localInput[3] = '\0';

        const char* shownInput = (inputLenLocal > 0) ? localInput : "---";
        const char gateChar = gateOpenLocal ? 'O' : 'C';

        if (sensorValidLocal) {
            printf_P(STR_LINE0, distanceCmLocal, thresholdCmLocal, gateChar);
        } else {
            printf_P(STR_LINE0_INVALID, thresholdCmLocal, gateChar);
        }
        printf_P(STR_LINE1, keyLocal, shownInput);
        printf_P(PSTR("\n"));

        vTaskDelay(pdMS_TO_TICKS(TASK_DISPLAY_PERIOD));
    }
}
