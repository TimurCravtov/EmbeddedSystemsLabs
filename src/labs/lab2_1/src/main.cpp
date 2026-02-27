#include <Arduino.h>
#include <stdio.h>

#include <button/button.h>
#include <led/led.h>
#include <serialio/serialio.h>

#include "DisplayTask.h"
#include "ReadingTask.h"
#include "Scheduler.h"
#include "StatisticsTask.h"
#include "Timer.h"


// leds
constexpr uint8_t redLedPin = 2;
constexpr uint8_t greenLedPin = 3;
constexpr uint8_t yellowPin = 4;

// button
constexpr uint8_t buttonPin = 5;

// led objects
Led redLed(redLedPin);
Led greenLed(greenLedPin);
Led yellowLed(yellowPin);
Button button(buttonPin);

// task config
namespace ReadingTask { uint16_t recurrenceDelay = 2; }
namespace StatisticsTask { uint16_t recurrenceDelay = 2; }
namespace DisplayTask { uint16_t recurrenceDelay = 10000; }

// offsets
namespace ReadingTask { uint16_t offset = 3; }
namespace StatisticsTask { uint16_t offset = 2; }
namespace DisplayTask { uint16_t offset = 10000; }

// add tasks
void setupScheduler() {
  Scheduler::addTask(StatisticsTask::run, StatisticsTask::recurrenceDelay,
                     StatisticsTask::offset);
  Scheduler::addTask(ReadingTask::run, ReadingTask::recurrenceDelay,
                     ReadingTask::offset);
  Scheduler::addTask(DisplayTask::run, DisplayTask::recurrenceDelay,
                     DisplayTask::offset);
  Scheduler::addTask(StatisticsLedTask::run, StatisticsLedTask::recurrenceDelay,
                     StatisticsLedTask::offset);
}

void setup() {
  redLed.init();
  greenLed.init();
  yellowLed.init();

  button.init();
  Timer::init();

  setupScheduler();

  Serial.begin(1000000);
  redirectSerialToStdio(true, true, true);
}

void loop() { 
    Scheduler::loop(); 
}
