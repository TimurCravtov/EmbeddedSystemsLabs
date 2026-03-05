#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Arduino.h>
#include <stdio.h>

#include <button/button.h>
#include <led/led.h>
#include <serialio/serialio.h>

#include "DisplayTask.h"
#include "ReadingTask.h"
#include "StatisticsTask.h"

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


// shared variables
extern SemaphoreHandle_t pressSemaphore;


void setup() {

    redLed.init();
    greenLed.init();
    yellowLed.init();
    button.init();

    Serial.begin(9600);

    redirectSerialToStdio(true, true, true);

    pressSemaphore = xSemaphoreCreateBinary();

    xTaskCreate(ReadingTask::run, "Reading", 128, NULL, 3, NULL);
    xTaskCreate(StatisticsTask::run, "Statistics", 128, NULL, 2, NULL);
    xTaskCreate(DisplayTask::run, "Display", 128, NULL, 1, NULL);

    vTaskStartScheduler();
}

void loop() {
}
