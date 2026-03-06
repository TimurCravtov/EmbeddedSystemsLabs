#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Arduino.h>
#include "ReadingTask.h"
#include <button/button.h>
#include <led/led.h>

namespace ReadingTask {
    uint16_t recurrenceDelay = 0;
    uint16_t offset = 0;
    uint32_t lastDuration = 0;
}

void visualizeButtonPressDuration();

SemaphoreHandle_t pressSemaphore;

extern Button button;
extern Led redLed;
extern Led greenLed;

extern const uint16_t PRESS_DURATION_THRESHOLD_MS;

void ReadingTask::run(void* parameters) {

    uint32_t startTime = 0;
    bool lastState = false;

    while (true) {
        bool isPressed = button.isPressed();

        // Button just pressed
        if (isPressed && !lastState) {
            startTime = millis();
        }

        // Button just released
        if (!isPressed && lastState && startTime != 0) {
            ReadingTask::lastDuration = millis() - startTime;
            startTime = 0;

            xSemaphoreGive(pressSemaphore);

            visualizeButtonPressDuration();
        }

        lastState = isPressed;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void visualizeButtonPressDuration() {

    if (ReadingTask::lastDuration < PRESS_DURATION_THRESHOLD_MS) {
        greenLed.on();
        redLed.off();
    } else {
        redLed.on();
        greenLed.off();
    }
}