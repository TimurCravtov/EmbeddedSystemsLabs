#include "sensors/distance.h"
#include <Arduino_FreeRTOS.h>

volatile uint32_t echoStart = 0;
volatile uint32_t echoEnd = 0;
volatile bool echoReady = false;
static uint8_t _echoPin;

void echoISR() {
    if (digitalRead(_echoPin)) echoStart = micros();
    else { echoEnd = micros(); echoReady = true; }
}

DistanceSensor::DistanceSensor(uint8_t triggerPin, uint8_t echoPin) 
    : triggerPin(triggerPin), echoPin(echoPin) { }

void DistanceSensor::_init() {
    _echoPin = this->echoPin;
    pinMode(this->triggerPin, OUTPUT);
    pinMode(this->echoPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(this->echoPin), echoISR, CHANGE);
}

float DistanceSensor::_readRaw() {
    echoReady = false;
    digitalWrite(this->triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(this->triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(this->triggerPin, LOW);

    TickType_t startTick = xTaskGetTickCount();
    while (!echoReady && (xTaskGetTickCount() - startTick) < pdMS_TO_TICKS(30)) {
        vTaskDelay(1);
    }

    if (!echoReady) return 0;
    uint32_t duration = echoEnd - echoStart;
    return duration * 0.034f / 2.0f;
}