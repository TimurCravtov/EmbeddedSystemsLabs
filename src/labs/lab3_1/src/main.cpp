#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Arduino.h>
#include <stdio.h>

#include <led/led.h>
#include <serialio/serialio.h>
#include <sensors/distance.h>

#include "GenericReadingSensorTask.h"
#include "ThresholdTask.h"
#include "ReportTask.h"

// --- Sensor configuration ---
const SensorConfig distanceConfig = {
    50,     // readingIntervalMs  (20-100 ms)
    100,    // thresholdIntervalMs
    25.0,   // thresholdCenter    (cm)
    2.0,    // hysteresis         (+- cm)
    3,      // debounceRequired
    "DIST"  // name
};

const uint16_t REPORT_RECURRENCE_MS = 500;

// --- Hardware pins ---
const uint8_t TRIGGER_PIN   = 9;
const uint8_t ECHO_PIN      = 10;
const uint8_t RED_LED_PIN   = 12;
const uint8_t GREEN_LED_PIN = 13;

// --- Objects ---
Led redLed(RED_LED_PIN);
Led greenLed(GREEN_LED_PIN);

DistanceSensor distanceSensor(TRIGGER_PIN, ECHO_PIN);
GenericReadingSensorTask<DistanceSensor> distanceTask(distanceSensor, distanceConfig);

// --- Shared sensor list (add more entries for a second sensor) ---
SensorData* sensorList[] = { distanceTask.getDataPtr() };
const uint8_t SENSOR_COUNT = sizeof(sensorList) / sizeof(sensorList[0]);

// --- Task 2: One ThresholdTask instance per sensor ---
ThresholdTask distanceThresholdTask(distanceTask.getDataPtr());

// --- Task 3: Report params ---
void onAlertUpdate(bool anyAlert) {
    if (anyAlert) {
        redLed.on();
        greenLed.off();
    } else {
        redLed.off();
        greenLed.on();
    }
}

ReportTaskParams reportParams = {
    sensorList,
    SENSOR_COUNT,
    REPORT_RECURRENCE_MS,
    onAlertUpdate
};

void setup() {
    Serial.begin(9600);
    redirectSerialToStdio(true, true, true);

    printf("[Setup] Init...\n");

    redLed.init();
    greenLed.init();
    greenLed.on();
    distanceSensor.init();

    xTaskCreate(
        GenericReadingSensorTask<DistanceSensor>::taskEntryPoint,
        "Acquire",
        96,
        &distanceTask,
        3,
        NULL
    );

    xTaskCreate(
        ThresholdTask::taskEntryPoint,
        "Thresh",
        96,
        &distanceThresholdTask,
        2,
        NULL
    );

    xTaskCreate(
        ReportTask::run,
        "Report",
        150,
        &reportParams,
        1,
        NULL
    );

    vTaskStartScheduler();
}

void loop() { }

