#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include "DisplayTask.h"
#include "StatisticsTask.h"

void DisplayTask::run(void* parameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(10 * 1000); // 10 seconds

    while (true) {
        vTaskDelayUntil(&lastWakeTime, interval);

        xSemaphoreTake(Statistics::statsMutex, portMAX_DELAY);

        uint16_t currentShort = Statistics::shortPressesNumber;
        uint16_t currentLong = Statistics::longPressesNumber;
        uint32_t currentShortDuration = Statistics::shortPressesTotalDuration;
        uint32_t currentLongDuration = Statistics::longPressesTotalDuration;

        Statistics::shortPressesNumber = 0;
        Statistics::longPressesNumber = 0;
        Statistics::shortPressesTotalDuration = 0;
        Statistics::longPressesTotalDuration = 0;

        xSemaphoreGive(Statistics::statsMutex);

        uint32_t totalCount = currentShort + currentLong;
        uint32_t totalDuration = currentShortDuration + currentLongDuration;

        if (totalCount > 0) {
            uint32_t avgMs = totalDuration / totalCount;
            uint32_t wholeSeconds = avgMs / 1000;
            uint32_t fractionalSeconds = avgMs % 1000;
            uint32_t decimals = fractionalSeconds / 10;

            printf_P(PSTR("L: %u, S: %u, Avg: %lu.%02lus\n\r"),
                currentLong,
                currentShort,
                (unsigned long)wholeSeconds,
                (unsigned long)decimals);
        } else {
            printf_P(PSTR("L: %u, S: %u, Avg: 0.00s\n\r"),
                currentLong,
                currentShort);
        }
    }
}


