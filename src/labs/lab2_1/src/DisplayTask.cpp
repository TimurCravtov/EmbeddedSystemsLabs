#include <stdio.h>
#include "DisplayTask.h"
#include "StatisticsTask.h"
#include <stdlib.h>
#include <Arduino.h>

void DisplayTask::run() {
    uint32_t totalCount = Statistics::shortPressesNumber + Statistics::longPressesNumber;
    uint32_t totalDuration = Statistics::shortPressesTotalDuration + Statistics::longPressesTotalDuration;
    
    if (totalCount > 0) {
        // Calculate average duration in pure milliseconds
        uint32_t avgMs = totalDuration / totalCount;
        
        // Extract whole seconds and the fractional remainder
        uint32_t wholeSeconds = avgMs / 1000;
        uint32_t fractionalSeconds = avgMs % 1000;
        
        // To get exactly 2 decimal places (e.g., .25s), divide the remainder by 10
        uint32_t decimals = fractionalSeconds / 10;
        
        // Use %lu (unsigned long) to ensure compatibility across all architectures
        printf_P(PSTR("L: %u, S: %u, Avg: %lu.%02lus\n"), 
               Statistics::longPressesNumber, 
               Statistics::shortPressesNumber, 
               (unsigned long)wholeSeconds, 
               (unsigned long)decimals);
    } else {
        // Fallback for before any buttons are pressed
        printf_P(PSTR("L: %u, S: %u, Avg: 0.00s\n"), 
               Statistics::longPressesNumber, 
               Statistics::shortPressesNumber);
    }

    // reset statistics after displaying
    Statistics::shortPressesNumber = Statistics::longPressesNumber = 0;
    Statistics::shortPressesTotalDuration = Statistics::longPressesTotalDuration = 0;

}