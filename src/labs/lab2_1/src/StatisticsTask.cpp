#include "StatisticsTask.h"
#include "ReadingTask.h"
#include <led/led.h>

// Global LED instances are defined in main.cpp
extern Led yellowLed;

namespace Statistics {
uint16_t shortPressesNumber = 0;
unsigned long shortPressesTotalDuration = 0;

uint16_t longPressesNumber = 0;
unsigned long longPressesTotalDuration = 0;

uint16_t lastHandledUpdateTime = -1;
} // namespace Statistics

void StatisticsTask::run() {

  // if there is a new update from the reading task, we update the statistics
  if (ReadingTask::lastUpdatedTime != Statistics::lastHandledUpdateTime) {
    Statistics::lastHandledUpdateTime = ReadingTask::lastUpdatedTime;

    if (ReadingTask::lastDuration > 0) {
      if (ReadingTask::lastDuration < 500 && ReadingTask::lastDuration > 0) {
        Statistics::shortPressesTotalDuration += ReadingTask::lastDuration;
        Statistics::shortPressesNumber++;
      } else {
        Statistics::longPressesTotalDuration += ReadingTask::lastDuration;
        Statistics::longPressesNumber++;
      }

      StatisticsLedTask::triggerBlinks(ReadingTask::lastDuration >= 500);
    }
  }
}

namespace StatisticsLedTask {

uint16_t recurrenceDelay = 2;
uint16_t offset = 2;

// use the global yellowLed defined in main.cpp

// Internal state tracking
int blinksRemaining = 0;
unsigned long lastActionTime = 0;
const int blinkInterval = 100;

void triggerBlinks(bool isLong) {
  ::yellowLed.off();         // Force off immediately
  lastActionTime = millis(); // Reset timer so it blinks 'on' instantly
  blinksRemaining = isLong ? 10 : 5;
}

void run() {
  if (blinksRemaining <= 0)
    return;

  unsigned long currentTime = millis();
  if (currentTime - lastActionTime >= blinkInterval) {
    lastActionTime = currentTime;

    // Toggle logic
    if (::yellowLed.isOn()) {
      ::yellowLed.off();
      blinksRemaining--; // Decrease count after a full on/off cycle
    } else {
      ::yellowLed.on();
    }
  }
}
} // namespace StatisticsLedTask