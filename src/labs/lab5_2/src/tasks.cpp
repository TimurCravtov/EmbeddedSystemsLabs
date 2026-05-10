#include "tasks.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
// PROGMEM strings — saves SRAM on AVR.
// ─────────────────────────────────────────────────────────────────────────────
static const char STR_LINE0[]         PROGMEM = "\rD:%3u T:%3u %c   \n";
static const char STR_LINE0_INVALID[] PROGMEM = "\rD:--- T:%3u %c   \n";
static const char STR_LINE1[]         PROGMEM = "\rK:%c In:%-3s      ";

// Plotter format strings in PROGMEM.
// Integer-only specifiers (%u / %d) keep the linker away from float printf.
static const char PLOT_DIST[]     PROGMEM = ">pos/dist:%d.%u\n";
static const char PLOT_SETPOINT[] PROGMEM = ">pos/setpoint:%u\n";
static const char PLOT_ERROR[]    PROGMEM = ">pid/error:%d.%u\n";
static const char PLOT_OUTPUT[]   PROGMEM = ">pid/output:%d\n";
static const char PLOT_VALID[]    PROGMEM = ">status/valid:%u\n";

// ─────────────────────────────────────────────────────────────────────────────
// Shared state
// ─────────────────────────────────────────────────────────────────────────────
volatile uint16_t    measuredDistanceCm = 0;
volatile uint16_t    minDistanceCm      = MIN_DISTANCE_DEFAULT_CM;
volatile uint8_t     servoAngle         = 0;
volatile int16_t     pidErrorCm         = 0;
volatile uint8_t     pidOutputPos       = SERVO_POS_NEUTRAL;
volatile bool        gateOpen           = false;
volatile bool        sensorValid        = false;
volatile char        lastKeyPressed     = '-';
volatile SystemState sysState           = {0, MIN_DISTANCE_DEFAULT_CM, 0, 0,
                                            SERVO_POS_NEUTRAL, false, false, '-'};

// ─────────────────────────────────────────────────────────────────────────────
// Keypad input buffer (max 3 digits, value <= 400)
// ─────────────────────────────────────────────────────────────────────────────
static char    inputBuffer[4] = {0};
static uint8_t inputIndex     = 0;

static inline void clearInputBuffer() {
    inputIndex     = 0;
    inputBuffer[0] = '\0';
}

static uint16_t parseInputBuffer() {
    uint16_t value = 0;
    for (uint8_t i = 0; i < inputIndex; i++) {
        value = (uint16_t)(value * 10u + (uint16_t)(inputBuffer[i] - '0'));
    }
    return value;
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility helpers
// ─────────────────────────────────────────────────────────────────────────────
static inline uint16_t clampThreshold(uint16_t value) {
    if (value < MIN_DISTANCE_MIN_CM) return MIN_DISTANCE_MIN_CM;
    if (value > MIN_DISTANCE_MAX_CM) return MIN_DISTANCE_MAX_CM;
    return value;
}

static inline float clampFloat(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static inline int16_t clampInt16(int16_t value, int16_t minValue, int16_t maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static inline int16_t roundToInt(float value) {
    return (int16_t)(value + ((value >= 0.0f) ? 0.5f : -0.5f));
}

// ─────────────────────────────────────────────────────────────────────────────
// Integer fixed-point EMA
//
// Values are stored x10 (tenths) so one decimal place prints as integers.
// Alpha = 1 / (1 << EMA_SHIFT):
//   shift 2 -> alpha ~0.25  (faster, less smooth)
//   shift 3 -> alpha ~0.125 (default, moderate)
//   shift 4 -> alpha ~0.063 (very smooth, more lag)
// ─────────────────────────────────────────────────────────────────────────────
#define EMA_SHIFT 3

static inline int16_t emaStep(int16_t smoothed, int16_t incoming) {
    return (int16_t)(smoothed + ((incoming - smoothed) >> EMA_SHIFT));
}

// Print a x10 fixed-point value with one decimal place using PROGMEM format.
// E.g. 173 -> "17.3",  -23 -> "-2.3"
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

// ─────────────────────────────────────────────────────────────────────────────
// PID state
// ─────────────────────────────────────────────────────────────────────────────
static float pidIntegral = 0.0f;
static float lastError   = 0.0f;

// ─────────────────────────────────────────────────────────────────────────────
// TaskRead
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
// TaskFilter — sensor read + PID control loop
// ─────────────────────────────────────────────────────────────────────────────
void TaskFilter(void* pvParameters) {
    ControlContext*  ctx      = static_cast<ControlContext*>(pvParameters);
    ServoActuator*   actuator = ctx->actuator;
    DistanceSensor*  sensor   = ctx->sensor;

    actuator->setPosition(SERVO_POS_NEUTRAL);
    vTaskDelay(pdMS_TO_TICKS(300));
    gateOpen   = false;
    servoAngle = actuator->getAngle();

    while (true) {
        const float rawDistance = sensor->readRaw();
        const bool  valid       = (rawDistance > 0.5f) &&
                                   (rawDistance <= (float)MIN_DISTANCE_MAX_CM);
        const uint16_t distanceCm = valid ? (uint16_t)(rawDistance + 0.5f) : 0u;

        if (valid) {
            const float setpoint   = (float)minDistanceCm;
            float error = setpoint - (float)distanceCm;
            if (PID_INVERT_OUTPUT) error = -error;

            const float dt  = (float)TASK_CONTROL_PERIOD / 1000.0f;
            pidIntegral      = clampFloat(pidIntegral + (error * dt),
                                          PID_INTEGRAL_MIN, PID_INTEGRAL_MAX);
            const float derivative = (error - lastError) / dt;
            const float control    = (PID_KP * error)
                                   + (PID_KI * pidIntegral)
                                   + (PID_KD * derivative);

            int16_t command = (int16_t)SERVO_POS_NEUTRAL + roundToInt(control);
            command = clampInt16(command, SERVO_POS_MIN, SERVO_POS_MAX);
            actuator->setPosition((uint8_t)command);

            gateOpen     = (command >= SERVO_POS_NEUTRAL);
            lastError    = error;
            pidErrorCm   = roundToInt(error);
            pidOutputPos = (uint8_t)command;
        } else {
            actuator->setPosition(SERVO_POS_NEUTRAL);
            gateOpen     = false;
            pidIntegral  = 0.0f;
            lastError    = 0.0f;
            pidErrorCm   = 0;
            pidOutputPos = SERVO_POS_NEUTRAL;
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

// ─────────────────────────────────────────────────────────────────────────────
// TaskDisplay — LCD text  OR  Teleplot serial plotter
//
// Teleplot channel layout
// ───────────────────────
//   pos/dist      — EMA-smoothed distance (cm, 1 d.p.)   ─┐ same graph
//   pos/setpoint  — target distance (cm, integer)          ┘
//   pid/error     — EMA-smoothed PID error (cm, 1 d.p.)
//   pid/output    — servo offset from neutral (integer servo units)
//   status/valid  — 1 = sensor OK, 0 = sensor lost
//
// Memory decisions
// ────────────────
//   • Format strings in PROGMEM  → saves 5 x ~20 bytes of SRAM.
//   • Only %d / %u specifiers    → linker uses small integer printf,
//                                   NOT the large float printf variant.
//   • EMA in fixed-point int16_t → no floats in this task whatsoever.
//   • EMA state: 3 x int16_t + 1 bool = 7 bytes total (vs 3 x float = 12).
// ─────────────────────────────────────────────────────────────────────────────
void TaskDisplay(void* pvParameters) {
    (void)pvParameters;

    // EMA state — x10 scale, int16_t (max 400 cm -> 4000, fits fine).
    static int16_t smoothDist_x10 = 0;
    static int16_t smoothErr_x10  = 0;
    static int16_t smoothOut      = 0;   // servo offset; no decimal needed
    static bool    smoothReady    = false;

    while (true) {
        // ── Capture shared state in one critical section ─────────────────────
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

        // ════════════════════════════════════════════════════════════════════
        //  TELEPLOT PLOTTER PATH
        // ════════════════════════════════════════════════════════════════════
        if (ENABLE_PLOTTER) {
            if (validLocal) {
                const int16_t dist_x10 = (int16_t)(dLocal * 10u);
                const int16_t err_x10  = (int16_t)(errLocal * 10);
                const int16_t out      = (int16_t)((int16_t)outLocal
                                                 - (int16_t)SERVO_POS_NEUTRAL);

                if (!smoothReady) {
                    // Seed on first valid reading — no startup spike
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
                // Dropout: reset smoother, keep setpoint visible, flag it
                smoothReady = false;
                printf_P(PLOT_SETPOINT, tLocal);
                printf_P(PLOT_VALID,    0u);
            }

            vTaskDelay(pdMS_TO_TICKS(TASK_PLOT_PERIOD));
            continue;
        }

        // ════════════════════════════════════════════════════════════════════
        //  LCD / SERIAL TEXT DISPLAY PATH  (unchanged from original)
        // ════════════════════════════════════════════════════════════════════
        char dirChar = '=';
        if (angleLocal > (uint8_t)(SERVO_ANGLE_NEUTRAL + SERVO_ANGLE_DEADBAND)) {
            dirChar = '>';
        } else if (angleLocal < (uint8_t)(SERVO_ANGLE_NEUTRAL - SERVO_ANGLE_DEADBAND)) {
            dirChar = '<';
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