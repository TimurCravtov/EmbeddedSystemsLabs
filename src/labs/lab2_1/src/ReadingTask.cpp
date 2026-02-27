#include "ReadingTask.h"
#include <button/button.h>
#include <led/led.h>
#include <stdint.h>

void visualizeButtonPressDuration();

namespace ReadingTask {
    extern uint16_t recurrenceDelay;
    extern uint16_t offset;
    unsigned long lastUpdatedTime = -1; 
    unsigned long lastDuration = 0;
}

extern Button button;

unsigned long currentDuration = 0; 

void ReadingTask::run() {

    bool isPressed = button.isPressed();
    bool itsFreshPress = currentDuration == 0;

    bool pressJustStopped = !isPressed && !itsFreshPress;

    if (pressJustStopped) {
        lastUpdatedTime = millis();
        ReadingTask::lastDuration = currentDuration;
        currentDuration = 0;
        visualizeButtonPressDuration();
    }

    if (isPressed) {
        currentDuration += ReadingTask::recurrenceDelay;
    }

}

void visualizeButtonPressDuration() {
    extern Led redLed;
    extern Led greenLed;

    if (ReadingTask::lastDuration < 500) {
        greenLed.on();
        redLed.off();
    } else {
        redLed.on();
        greenLed.off();
    }
}