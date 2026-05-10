#include "tasks.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
// PROGMEM strings — padded to 16 chars to fully overwrite LCD leftovers.
// ─────────────────────────────────────────────────────────────────────────────
static const char STR_LINE0[]         PROGMEM = "\rD:%3u T:%3u %c   \n";
static const char STR_LINE0_INVALID[] PROGMEM = "\rD:--- T:%3u %c   \n";
static const char STR_LINE1[]         PROGMEM = "\rK:%c In:%-3s      ";

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
// Keypad input buffer (max 3 digits, value ≤ 400)
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
// PID state
// ─────────────────────────────────────────────────────────────────────────────
static float pidIntegral = 0.0f;
static float lastError   = 0.0f;

// ─────────────────────────────────────────────────────────────────────────────
// TaskRead — receives keypad / serial commands
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
                    minDistanceCm       = clampThreshold(parseInputBuffer());
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
                // Quick +5 cm step
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
            const float setpoint = (float)minDistanceCm;
            float error = setpoint - (float)distanceCm;
            if (PID_INVERT_OUTPUT) error = -error;

            const float dt = (float)TASK_CONTROL_PERIOD / 1000.0f;
            pidIntegral     = clampFloat(pidIntegral + (error * dt),
                                         PID_INTEGRAL_MIN, PID_INTEGRAL_MAX);
            const float derivative = (error - lastError) / dt;
            const float control    = (PID_KP * error)
                                   + (PID_KI * pidIntegral)
                                   + (PID_KD * derivative);

            int16_t command = (int16_t)SERVO_POS_NEUTRAL + roundToInt(control);
            command = clampInt16(command, SERVO_POS_MIN, SERVO_POS_MAX);
            actuator->setPosition((uint8_t)command);

            gateOpen  = (command >= SERVO_POS_NEUTRAL);
            lastError = error;
            pidErrorCm  = roundToInt(error);
            pidOutputPos = (uint8_t)command;
        } else {
            actuator->setPosition(SERVO_POS_NEUTRAL);
            gateOpen     = false;
            pidIntegral  = 0.0f;
            lastError    = 0.0f;
            pidErrorCm   = 0;
            pidOutputPos = SERVO_POS_NEUTRAL;
        }

        measuredDistanceCm = distanceCm;
        sensorValid        = valid;
        servoAngle         = actuator->getAngle();

        // Update shared snapshot atomically
        sysState.distanceCm   = measuredDistanceCm;
        sysState.thresholdCm  = minDistanceCm;
        sysState.angle        = servoAngle;
        sysState.errorCm      = pidErrorCm;
        sysState.outputPos    = pidOutputPos;
        sysState.isOpen       = gateOpen;
        sysState.isSensorValid = sensorValid;

        vTaskDelay(pdMS_TO_TICKS(TASK_CONTROL_PERIOD));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TaskDisplay — LCD output  OR  Teleplot serial plotter
//
// Teleplot channel layout
// ───────────────────────
//   pos/dist      — EMA-smoothed measured distance (cm)
//   pos/setpoint  — target distance (cm)          ← same axis as dist
//   pid/error     — EMA-smoothed PID error (cm)
//   pid/output    — EMA-smoothed servo offset from neutral (servo units)
//   status/valid  — 1 = sensor OK, 0 = sensor lost
//
// Grouping trick: channels that share a "/" prefix are plotted on the
// same Teleplot graph automatically, so dist vs. setpoint is immediately
// visible without any manual axis-linking.
//
// EMA smoothing
// ─────────────
//   smoothed = ALPHA * new + (1 - ALPHA) * smoothed
//   ALPHA = 0.20 → moderate smoothing, ~5-sample lag.
//   Increase toward 0.40 for a tighter/faster response.
//   Decrease toward 0.08 for a glassier trace.
// ─────────────────────────────────────────────────────────────────────────────
void TaskDisplay(void* pvParameters) {
    (void)pvParameters;

    // ── EMA filter state (persists across loop iterations) ──────────────────
    static float smoothDist  = 0.0f;
    static float smoothErr   = 0.0f;
    static float smoothOut   = 0.0f;
    static bool  smoothReady = false;

    // Tune this to taste (see notes above)
    constexpr float ALPHA = 0.20f;

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

        // ── Sanitise key for display ─────────────────────────────────────────
        if (keyLocal < 32 || keyLocal > 126) keyLocal = '-';

        // ════════════════════════════════════════════════════════════════════
        //  TELEPLOT PLOTTER PATH
        // ════════════════════════════════════════════════════════════════════
        if (ENABLE_PLOTTER) {
            if (validLocal) {
                const float fDist = (float)dLocal;
                const float fErr  = (float)errLocal;
                // Centre servo output around neutral so the trace is 0-based
                const float fOut  = (float)((int16_t)outLocal
                                          - (int16_t)SERVO_POS_NEUTRAL);

                // Seed the smoother on first valid reading (avoids a big
                // initial spike from 0 → real value)
                if (!smoothReady) {
                    smoothDist  = fDist;
                    smoothErr   = fErr;
                    smoothOut   = fOut;
                    smoothReady = true;
                } else {
                    smoothDist = ALPHA * fDist + (1.0f - ALPHA) * smoothDist;
                    smoothErr  = ALPHA * fErr  + (1.0f - ALPHA) * smoothErr;
                    smoothOut  = ALPHA * fOut  + (1.0f - ALPHA) * smoothOut;
                }

                printf(">pos/dist:%.1f\n"
                       ">pos/setpoint:%u\n"
                       ">pid/error:%.1f\n"
                       ">pid/output:%.1f\n"
                       ">status/valid:1\n",
                       smoothDist,
                       tLocal,
                       smoothErr,
                       smoothOut);
            } else {
                // Sensor lost: reset smoother so next valid reading seeds
                // cleanly; keep setpoint visible so the operator can still
                // see the target, and flag the dropout.
                smoothReady = false;

                printf(">pos/setpoint:%u\n"
                       ">status/valid:0\n",
                       tLocal);
            }

            vTaskDelay(pdMS_TO_TICKS(TASK_PLOT_PERIOD));
            continue;
        }

        // ════════════════════════════════════════════════════════════════════
        //  LCD / SERIAL TEXT DISPLAY PATH
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