#include "tasks.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

// progmem strings
static const char STR_LINE0[]         PROGMEM = "\rD:%3u T:%3u %c   \n";
static const char STR_LINE0_INVALID[] PROGMEM = "\rD:--- T:%3u %c   \n";
static const char STR_LINE1[]         PROGMEM = "\rK:%c In:%-3s      ";

// plotter format strings
static const char PLOT_DIST[]     PROGMEM = ">pos/dist:%d.%u\n";
static const char PLOT_SETPOINT[] PROGMEM = ">pos/setpoint:%u\n";
static const char PLOT_ERROR[]    PROGMEM = ">pid/error:%d.%u\n";
static const char PLOT_OUTPUT[]   PROGMEM = ">pid/output:%d\n";
static const char PLOT_VALID[]    PROGMEM = ">status/valid:%u\n";

// shared state variables
volatile uint16_t    measuredDistanceCm = 0;
volatile uint16_t    minDistanceCm      = MIN_DISTANCE_DEFAULT_CM;
volatile uint8_t     servoAngle         = 0;
volatile int16_t     pidErrorCm         = 0;
volatile uint8_t     pidOutputPos       = SERVO_POS_MIN;
volatile bool        gateOpen           = false;
volatile bool        sensorValid        = false;
volatile char        lastKeyPressed     = '-';
volatile SystemState sysState           = {0, MIN_DISTANCE_DEFAULT_CM, 0, 0,
                                           SERVO_POS_MIN, false, false, '-'};

// keypad input buffer
static char    inputBuffer[4] = {0};
static uint8_t inputIndex     = 0;

// resets the input buffer
static inline void clearInputBuffer() {
    inputIndex     = 0;
    inputBuffer[0] = '\0';
}

// converts the buffer to an integer
static uint16_t parseInputBuffer() {
    uint16_t value = 0;
    for (uint8_t i = 0; i < inputIndex; i++) {
        value = (uint16_t)(value * 10u + (uint16_t)(inputBuffer[i] - '0'));
    }
    return value;
}

// bounds the threshold value
static inline uint16_t clampThreshold(uint16_t value) {
    if (value < MIN_DISTANCE_MIN_CM) return MIN_DISTANCE_MIN_CM;
    if (value > MIN_DISTANCE_MAX_CM) return MIN_DISTANCE_MAX_CM;
    return value;
}

// bounds a float value
static inline float clampFloat(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

// bounds an int16 value
static inline int16_t clampInt16(int16_t value, int16_t minValue, int16_t maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

// rounds a float to the nearest int16
static inline int16_t roundToInt(float value) {
    return (int16_t)(value + ((value >= 0.0f) ? 0.5f : -0.5f));
}

// fixed point moving average shift
#define EMA_SHIFT 3

// calculates exponential moving average step
static inline int16_t emaStep(int16_t smoothed, int16_t incoming) {
    return (int16_t)(smoothed + ((incoming - smoothed) >> EMA_SHIFT));
}

// prints fixed point value with one decimal
static void printFixed(const char* fmt_P, int16_t val_x10) {
    int16_t intPart;
    uint8_t fracPart;
    if (val_x10 < 0) {
        int16_t abs_x10 = (int16_t)(-val_x10);
        intPart  = (int16_t)(-(abs_x10 / 10));
        fracPart = (uint8_t) ( abs_x10 % 10u);
    } else {
        intPart  = (int16_t)(val_x10 / 10);
        fracPart = (uint8_t) (val_x10 % 10u);
    }
    printf_P(fmt_P, (int)intPart, (unsigned)fracPart);
}

// pid variables
static float pidIntegral = 0.0f;
static float lastError   = 0.0f;

// reads serial inputs for system configuration
void TaskRead(void* pvParameters) {
    (void)pvParameters;
    while (true) {
        char c = 0;
        if (scanf("%c", &c) > 0 && c != '\0' && c != '\n' && c != '\r') {
            lastKeyPressed = c;
            sysState.key   = c;

            if (c >= '0' && c <= '9') {
                if (inputIndex < 3) {
                    inputBuffer[inputIndex++] = c;
                    inputBuffer[inputIndex]   = '\0';
                }
            } else if (c == '#' || c == 'A') {
                if (inputIndex > 0) {
                    minDistanceCm        = clampThreshold(parseInputBuffer());
                    sysState.thresholdCm = minDistanceCm;
                    clearInputBuffer();
                }
            } else if (c == '*' || c == 'B') {
                clearInputBuffer();
            } else if (c == 'C') {
                minDistanceCm        = MIN_DISTANCE_DEFAULT_CM;
                sysState.thresholdCm = minDistanceCm;
                clearInputBuffer();
            } else if (c == 'D') {
                minDistanceCm        = clampThreshold((uint16_t)(minDistanceCm + 5u));
                sysState.thresholdCm = minDistanceCm;
                clearInputBuffer();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_READ_PERIOD));
    }
}

// executes sensor reading and pid control
void TaskFilter(void* pvParameters) {
    ControlContext*  ctx      = static_cast<ControlContext*>(pvParameters);
    ServoActuator*   actuator = ctx->actuator;
    DistanceSensor*  sensor   = ctx->sensor;

    // start fully closed
    actuator->setPosition(SERVO_POS_MIN);
    vTaskDelay(pdMS_TO_TICKS(300));
    gateOpen   = false;
    servoAngle = actuator->getAngle();

    static bool controlActive   = false;
    static uint32_t lastOpenMs  = 0;

    while (true) {
        const float rawDistance = sensor->readRaw();
        const bool  valid       = (rawDistance > 0.5f) &&
                                  (rawDistance <= (float)MIN_DISTANCE_MAX_CM);
        const uint16_t distanceCm = valid ? (uint16_t)(rawDistance + 0.5f) : 0u;

        const uint32_t nowMs = millis();
        const bool inRange = valid && (distanceCm <= minDistanceCm);
        const bool aboveClose = valid &&
            (distanceCm > (uint16_t)(minDistanceCm + GATE_HYSTERESIS_CM));
        const bool holdOpen = controlActive &&
            ((uint32_t)(nowMs - lastOpenMs) < GATE_MIN_OPEN_MS);

        if (inRange) {
            if (!controlActive) {
                controlActive = true;
                lastOpenMs = nowMs;
            }
        } else if (!valid || aboveClose) {
            if (controlActive && (uint32_t)(nowMs - lastOpenMs) >= GATE_MIN_OPEN_MS) {
                controlActive = false;
            }
        }

        // runs PID only while active and in range; holds last output otherwise
        if (controlActive && inRange) {
            const float setpoint = (float)minDistanceCm;
            float error = setpoint - (float)distanceCm;

            // zeroes error near threshold to stop micro-movements;
            // integral is preserved so the servo can still reach SERVO_POS_MAX
            if (error >= -1.0f && error <= 1.0f) {
                error = 0.0f;
            }

            if (PID_INVERT_OUTPUT) error = -error;

            const float dt = (float)TASK_CONTROL_PERIOD / 1000.0f;
            pidIntegral    = clampFloat(pidIntegral + (error * dt),
                                        PID_INTEGRAL_MIN, PID_INTEGRAL_MAX);
            const float derivative = (error - lastError) / dt;
            float control  = (PID_KP * error)
                                 + (PID_KI * pidIntegral)
                                 + (PID_KD * derivative);

            // zeroes out negligible control signals to keep servo still
            if (control > -2.0f && control < 2.0f) {
                control = 0.0f;
            }

            // command offset from SERVO_POS_MIN: zero control = fully closed,
            // positive control opens toward SERVO_POS_MAX
            int16_t command = (int16_t)SERVO_POS_MIN + roundToInt(control);
            command = clampInt16(command, SERVO_POS_MIN, SERVO_POS_MAX);
            if (holdOpen && command < SERVO_POS_OPEN) {
                command = SERVO_POS_OPEN;
            }
            actuator->setPosition((uint8_t)command);

            gateOpen     = (command > SERVO_POS_MIN);
            lastError    = error;
            pidErrorCm   = roundToInt(error);
            pidOutputPos = (uint8_t)command;
        } else if (controlActive) {
            if (holdOpen) {
                actuator->setPosition(SERVO_POS_OPEN);
                gateOpen = true;
                pidOutputPos = SERVO_POS_OPEN;
            } else {
                actuator->setPosition(pidOutputPos);
                gateOpen = (pidOutputPos > SERVO_POS_MIN);
            }
        } else {
            // object out of range - drive fully closed, clear pid memory
            actuator->setPosition(SERVO_POS_MIN);
            gateOpen     = false;
            pidIntegral  = 0.0f;
            lastError    = 0.0f;
            pidErrorCm   = 0;
            pidOutputPos = SERVO_POS_MIN;
        }

        measuredDistanceCm  = distanceCm;
        sensorValid         = valid;
        servoAngle          = actuator->getAngle();

        sysState.distanceCm    = measuredDistanceCm;
        sysState.thresholdCm   = minDistanceCm;
        sysState.angle         = servoAngle;
        sysState.errorCm       = pidErrorCm;
        sysState.outputPos     = pidOutputPos;
        sysState.isOpen        = gateOpen;
        sysState.isSensorValid = sensorValid;

        vTaskDelay(pdMS_TO_TICKS(TASK_CONTROL_PERIOD));
    }
}

// displays output on serial and processes telemetry
void TaskDisplay(void* pvParameters) {
    (void)pvParameters;

    // moving average variables
    static int16_t smoothDist_x10 = 0;
    static int16_t smoothErr_x10  = 0;
    static int16_t smoothOut      = 0;
    static bool    smoothReady    = false;

    while (true) {
        // reads shared state
        uint16_t dLocal, tLocal;
        uint8_t  angleLocal, outLocal;
        int16_t  errLocal;
        bool     validLocal;
        char     keyLocal;
        char     localInput[4] = "---";

        taskENTER_CRITICAL();
        dLocal     = sysState.distanceCm;
        tLocal     = sysState.thresholdCm;
        angleLocal = sysState.angle;
        errLocal   = sysState.errorCm;
        outLocal   = sysState.outputPos;
        validLocal = sysState.isSensorValid;
        keyLocal   = sysState.key;
        if (inputIndex > 0) {
            strncpy(localInput, inputBuffer, 3);
            localInput[3] = '\0';
        }
        taskEXIT_CRITICAL();

        if (keyLocal < 32 || keyLocal > 126) keyLocal = '-';

        // teleplot serial logic
        if (ENABLE_PLOTTER) {
            if (validLocal) {
                const int16_t dist_x10 = (int16_t)(dLocal * 10u);
                const int16_t err_x10  = (int16_t)(errLocal * 10);
                // output relative to SERVO_POS_MIN: 0 = closed, positive = opening
                const int16_t out = (int16_t)((int16_t)outLocal
                                     - (int16_t)SERVO_POS_MIN);

                if (!smoothReady) {
                    smoothDist_x10 = dist_x10;
                    smoothErr_x10  = err_x10;
                    smoothOut      = out;
                    smoothReady    = true;
                } else {
                    smoothDist_x10 = emaStep(smoothDist_x10, dist_x10);
                    smoothErr_x10  = emaStep(smoothErr_x10,  err_x10);
                    smoothOut      = emaStep(smoothOut,       out);
                }

                printFixed(PLOT_DIST,  smoothDist_x10);
                printf_P(PLOT_SETPOINT, tLocal);
                printFixed(PLOT_ERROR, smoothErr_x10);
                printf_P(PLOT_OUTPUT,  (int)smoothOut);
                printf_P(PLOT_VALID,   1u);

            } else {
                smoothReady = false;
                printf_P(PLOT_SETPOINT, tLocal);
                printf_P(PLOT_VALID,    0u);
            }

            vTaskDelay(pdMS_TO_TICKS(TASK_PLOT_PERIOD));
            continue;
        }

        // text display logic
        // '=' closed (at or near MIN), '>' opening above MIN + deadband
        // '<' removed - servo never goes below SERVO_POS_MIN
        char dirChar = '=';
        if (angleLocal > (uint8_t)(SERVO_ANGLE_MIN + SERVO_ANGLE_DEADBAND)) {
            dirChar = '>';
        }

        if (validLocal) {
            printf_P(STR_LINE0, dLocal, tLocal, dirChar);
        } else {
            printf_P(STR_LINE0_INVALID, tLocal, dirChar);
        }

        printf_P(STR_LINE1, keyLocal, localInput);
        printf_P(PSTR("\n\n"));

        vTaskDelay(pdMS_TO_TICKS(TASK_DISPLAY_PERIOD));
    }
}