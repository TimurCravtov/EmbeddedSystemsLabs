#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "StatisticsTask.h"
#include "ReadingTask.h"
#include <led/led.h>

namespace Statistics {
    uint16_t shortPressesNumber = 0;
    unsigned long shortPressesTotalDuration = 0;
    uint16_t longPressesNumber = 0;
    unsigned long longPressesTotalDuration = 0;
    SemaphoreHandle_t statsMutex = xSemaphoreCreateMutex();
}

extern SemaphoreHandle_t pressSemaphore;
extern Led yellowLed;

void StatisticsTask::run(void* parameters) {

    while (true) {

        if (xSemaphoreTake(pressSemaphore, portMAX_DELAY) == pdTRUE) {
            uint32_t duration = ReadingTask::lastDuration;

            xSemaphoreTake(Statistics::statsMutex, portMAX_DELAY);

            if (duration < 500) {
                Statistics::shortPressesNumber++;
                Statistics::shortPressesTotalDuration += duration;
            } else {
                Statistics::longPressesNumber++;
                Statistics::longPressesTotalDuration += duration;
            }

            xSemaphoreGive(Statistics::statsMutex);

            int blinkCount = (duration >= 500) ? 10 : 5;
            for (int i = 0; i < blinkCount; i++) {
                yellowLed.on();
                vTaskDelay(pdMS_TO_TICKS(100));
                yellowLed.off();
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            
        }
    }
}
