#include "tasks.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <serialio/serialio.h>
#include <lcd/LcdStdioManager.h>
#include <avr/pgmspace.h>

// PROGMEM string literals to save RAM (for 16x2 LCD)
// \r returns to start of line, \n moves to next line
static const char STR_LINE0[] PROGMEM = "\rOut:%3d%% A:%3d\n";  // \r for overwrite, \n to go to line 1
static const char STR_OK[] PROGMEM = "\rStatus: OK      ";
static const char STR_LIMIT[] PROGMEM = "\r!LIMIT REACHED!";
static const char STR_OVERLOAD[] PROGMEM = "\r!OVERLOAD!     ";

// Shared state variables
volatile uint8_t rawPosition = 0;
volatile uint8_t filteredPosition = 0;
volatile uint8_t outputPosition = 0;
volatile uint8_t outputAngle = 0;
volatile bool commandReady = false;
volatile bool limitReached = false;
volatile bool overloadDetected = false;
volatile SystemState sysState = {0, 0, 0, 0, false, false};

// Median filter (window=5)
static uint8_t medianBuffer[5];
static uint8_t medianIndex = 0;

// Weighted averaging (weights 5,3,2 sum=10)
static uint8_t historyBuffer[3] = {0, 0, 0};

// Input buffer
static char inputBuffer[4] = {0};
static uint8_t inputIndex = 0;

// Insertion sort for small arrays (more efficient than bubble)
static void sortArray(uint8_t* arr, uint8_t size) {
    for (uint8_t i = 1; i < size; i++) {
        uint8_t key = arr[i];
        int8_t j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// Task 1: Read commands from keypad
void TaskRead(void* pvParameters) {
    (void)pvParameters;
    char c;
    
    while (true) {
        c = 0;
        if (scanf("%c", &c) > 0 && c > '\r') {
            if (c >= '0' && c <= '9') {
                if (inputIndex < 3) {
                    inputBuffer[inputIndex++] = c;
                    inputBuffer[inputIndex] = '\0';
                }
            } else if (c == '#' || c == 'A') {
                if (inputIndex > 0) {
                    uint8_t value = (uint8_t)atoi(inputBuffer);
                    if (value > 100) { value = 100; limitReached = true; }
                    else { limitReached = false; }
                    rawPosition = value;
                    commandReady = true;
                    inputIndex = 0;
                    inputBuffer[0] = '\0';
                }
            } else if (c == '*' || c == 'B') {
                inputIndex = 0;
                inputBuffer[0] = '\0';
            } else if (c == 'C') {
                rawPosition = 0;
                commandReady = true;
                overloadDetected = true;
                inputIndex = 0;
                inputBuffer[0] = '\0';
            } else if (c == 'D') {
                rawPosition = 100;
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
    ServoActuator* actuator = static_cast<ServoActuator*>(pvParameters);
    
    // Initialize buffers
    for (uint8_t i = 0; i < 5; i++) medianBuffer[i] = 0;
    historyBuffer[0] = historyBuffer[1] = historyBuffer[2] = 0;
    
    while (true) {
        // 1. Saturation
        uint8_t saturated = (rawPosition > 100) ? 100 : rawPosition;
        
        // 2. Median filter
        medianBuffer[medianIndex] = saturated;
        medianIndex = (medianIndex + 1) % 5;
        
        uint8_t sorted[5];
        for (uint8_t i = 0; i < 5; i++) sorted[i] = medianBuffer[i];
        sortArray(sorted, 5);
        uint8_t medianValue = sorted[2];
        
        // 3. Weighted average (weights: 5,3,2)
        historyBuffer[2] = historyBuffer[1];
        historyBuffer[1] = historyBuffer[0];
        historyBuffer[0] = medianValue;
        filteredPosition = (historyBuffer[0] * 5 + historyBuffer[1] * 3 + historyBuffer[2] * 2) / 10;
        
        // 4. Ramping (step=5)
        int8_t diff = (int8_t)filteredPosition - (int8_t)outputPosition;
        if (diff > 5) outputPosition += 5;
        else if (diff < -5) outputPosition -= 5;
        else outputPosition = filteredPosition;
        
        // 5. Apply to actuator
        actuator->setPosition(outputPosition);
        outputAngle = actuator->getAngle();
        
        // Update state
        sysState.raw = rawPosition;
        sysState.filtered = filteredPosition;
        sysState.output = outputPosition;
        sysState.angle = outputAngle;
        sysState.limitAlert = limitReached;
        sysState.overloadAlert = overloadDetected;
        
        vTaskDelay(pdMS_TO_TICKS(TASK_FILTER_PERIOD));
    }
}

// Task 3: Display on LCD (16x2, static lines)
// LCD driver: \n toggles row (0<->1), \r goes to column 0 of current row
void TaskDisplay(void* pvParameters) {
    (void)pvParameters;
    
    while (true) {
        // Ensure we start on line 0 (toggle if on line 1)
        // Print line 0 with \n to go to line 1
        printf_P(STR_LINE0, sysState.output, sysState.angle);
        
        // Now on line 1 - print status/alert
        if (sysState.overloadAlert) {
            printf_P(STR_OVERLOAD);
            overloadDetected = false;
        } else if (sysState.limitAlert) {
            printf_P(STR_LIMIT);
        } else {
            printf_P(STR_OK);
        }
        
        // Print \n to go back to line 0 for next cycle
        printf_P(PSTR("\n"));
        
        vTaskDelay(pdMS_TO_TICKS(TASK_DISPLAY_PERIOD));
    }
}
