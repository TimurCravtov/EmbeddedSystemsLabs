#include "tasks.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <serialio/serialio.h>
#include <lcd/LcdStdioManager.h>

// Shared state variables
volatile uint8_t rawSpeed = 0;
volatile uint8_t filteredSpeed = 0;
volatile uint8_t outputSpeed = 0;
volatile bool commandReady = false;
volatile bool limitReached = false;
volatile bool overloadDetected = false;
volatile SystemState sysState = {0, 0, 0, false, false};

// Median filter buffer
constexpr uint8_t MEDIAN_SIZE = 5;
static uint8_t medianBuffer[MEDIAN_SIZE];
static uint8_t medianIndex = 0;

// Weighted averaging coefficients (sum = 10)
constexpr uint8_t WEIGHTS[3] = {5, 3, 2};  // Recent values weighted more
static uint8_t historyBuffer[3] = {0, 0, 0};

// Ramping step per cycle
constexpr uint8_t RAMP_STEP = 5;

// Input buffer for multi-digit input
static char inputBuffer[4] = {0};
static uint8_t inputIndex = 0;

// Utility: Sort array for median calculation
static void sortArray(uint8_t* arr, uint8_t size) {
    for (uint8_t i = 0; i < size - 1; i++) {
        for (uint8_t j = i + 1; j < size; j++) {
            if (arr[i] > arr[j]) {
                uint8_t temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
}

// Task 1: Read commands from keypad
void TaskRead(void* pvParameters) {
    while (true) {
        char c = 0;
        int result = scanf("%c", &c);
        
        if (result > 0 && c != '\0' && c != '\n' && c != '\r') {
            if (c >= '0' && c <= '9') {
                // Accumulate digits for multi-digit value
                if (inputIndex < 3) {
                    inputBuffer[inputIndex++] = c;
                    inputBuffer[inputIndex] = '\0';
                }
            } else if (c == '#' || c == 'A') {
                // Confirm input (# or A key)
                if (inputIndex > 0) {
                    int value = atoi(inputBuffer);
                    // Validate: clamp to 0-100
                    if (value > 100) {
                        value = 100;
                        limitReached = true;
                    } else {
                        limitReached = false;
                    }
                    rawSpeed = (uint8_t)value;
                    commandReady = true;
                    
                    // Reset buffer
                    inputIndex = 0;
                    inputBuffer[0] = '\0';
                }
            } else if (c == '*' || c == 'B') {
                // Cancel input (* or B key)
                inputIndex = 0;
                inputBuffer[0] = '\0';
            } else if (c == 'C') {
                // Emergency stop
                rawSpeed = 0;
                commandReady = true;
                overloadDetected = true;
                inputIndex = 0;
                inputBuffer[0] = '\0';
            } else if (c == 'D') {
                // Full speed
                rawSpeed = 100;
                commandReady = true;
                limitReached = true;
                inputIndex = 0;
                inputBuffer[0] = '\0';
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(TASK_READ_PERIOD));
    }
}

// Task 2: Filter and condition signal
void TaskFilter(void* pvParameters) {
    PwmActuator* actuator = static_cast<PwmActuator*>(pvParameters);
    
    // Initialize median buffer
    for (uint8_t i = 0; i < MEDIAN_SIZE; i++) {
        medianBuffer[i] = rawSpeed;
    }
    historyBuffer[0] = historyBuffer[1] = historyBuffer[2] = rawSpeed;
    
    while (true) {
        // 1. Saturation (already done in input validation, but ensure here)
        uint8_t saturated = rawSpeed;
        if (saturated > ACTUATOR_MAX) saturated = ACTUATOR_MAX;
        if (saturated < ACTUATOR_MIN) saturated = ACTUATOR_MIN;
        
        // 2. Median filter (remove impulse noise)
        medianBuffer[medianIndex] = saturated;
        medianIndex = (medianIndex + 1) % MEDIAN_SIZE;
        
        uint8_t sortedBuffer[MEDIAN_SIZE];
        for (uint8_t i = 0; i < MEDIAN_SIZE; i++) {
            sortedBuffer[i] = medianBuffer[i];
        }
        sortArray(sortedBuffer, MEDIAN_SIZE);
        uint8_t medianValue = sortedBuffer[MEDIAN_SIZE / 2];
        
        // 3. Weighted average (reduce fluctuations)
        historyBuffer[2] = historyBuffer[1];
        historyBuffer[1] = historyBuffer[0];
        historyBuffer[0] = medianValue;
        
        uint16_t weightedSum = historyBuffer[0] * WEIGHTS[0] 
                             + historyBuffer[1] * WEIGHTS[1] 
                             + historyBuffer[2] * WEIGHTS[2];
        filteredSpeed = weightedSum / 10;  // Divide by sum of weights
        
        // 4. Ramping (soft start/stop)
        int16_t diff = (int16_t)filteredSpeed - (int16_t)outputSpeed;
        if (diff > RAMP_STEP) {
            outputSpeed += RAMP_STEP;
        } else if (diff < -RAMP_STEP) {
            outputSpeed -= RAMP_STEP;
        } else {
            outputSpeed = filteredSpeed;
        }
        
        // 5. Apply to actuator
        actuator->setSpeed(outputSpeed);
        
        // Update system state
        sysState.raw = rawSpeed;
        sysState.filtered = filteredSpeed;
        sysState.output = outputSpeed;
        sysState.limitAlert = limitReached;
        sysState.overloadAlert = overloadDetected;
        
        vTaskDelay(pdMS_TO_TICKS(TASK_FILTER_PERIOD));
    }
}

// Task 3: Display on LCD
void TaskDisplay(void* pvParameters) {
    while (true) {
        // Clear and display structured report
        printf("\x1B[2J\x1B[H");  // Clear screen (ANSI escape)
        
        printf("=== ACTUATOR REPORT ===\n");
        printf("Raw:      %3d%%\n", sysState.raw);
        printf("Filtered: %3d%%\n", sysState.filtered);
        printf("Output:   %3d%%\n", sysState.output);
        printf("-----------------------\n");
        
        // Status and alerts
        if (sysState.overloadAlert) {
            printf("ALERT: OVERLOAD!\n");
            overloadDetected = false;  // Reset after display
        } else if (sysState.limitAlert) {
            printf("ALERT: LIMIT REACHED\n");
        } else {
            printf("Status: NORMAL\n");
        }
        
        printf("-----------------------\n");
        printf("Keys: 0-100+#=Set C=Stop\n");
        
        vTaskDelay(pdMS_TO_TICKS(TASK_DISPLAY_PERIOD));
    }
}
