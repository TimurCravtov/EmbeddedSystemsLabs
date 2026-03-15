#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Arduino.h>
#include <stdio.h>

#include <led/led.h>
#include <serialio/serialio.h>
#include <sensors/distance.h>
#include <sensors/temperature.h>
#include <LiquidCrystal_I2C.h>
#include <lcd/LcdStdioManager.h>

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

const SensorConfig tempConfig = {
    100,    // readingIntervalMs
    200,    // thresholdIntervalMs
    25.0,   // thresholdCenter    (Celsius)
    2.0,    // hysteresis         (+- Celsius)
    3,      // debounceRequired
    "TEMP"  // name
};

float convertNtcAdcToCelsius(float adcValue) {
    if (adcValue <= 0 || adcValue >= 1023) return 0;
    const float BETA = 3950.0f;
    return 1.0f / (log(1.0f / (1023.0f / adcValue - 1.0f)) / BETA + 1.0f / 298.15f) - 273.15f;
}

const uint16_t REPORT_RECURRENCE_MS = 500;

// --- Hardware pins ---
const uint8_t TRIGGER_PIN   = 9;
const uint8_t ECHO_PIN      = 10;
const uint8_t TEMP_PIN      = A0;
const uint8_t RED_LED_PIN   = 12;
const uint8_t GREEN_LED_PIN = 13;

// --- Objects ---
Led redLed(RED_LED_PIN);
Led greenLed(GREEN_LED_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

DistanceSensor distanceSensor(TRIGGER_PIN, ECHO_PIN);
GenericReadingSensorTask<DistanceSensor> distanceTask(distanceSensor, distanceConfig);

TemperatureSensor temperatureSensor(TEMP_PIN);
GenericReadingSensorTask<TemperatureSensor> tempTask(temperatureSensor, tempConfig, convertNtcAdcToCelsius);

SensorData* sensorList[] = { distanceTask.getDataPtr(), tempTask.getDataPtr() };
const uint8_t SENSOR_COUNT = sizeof(sensorList) / sizeof(sensorList[0]);

ThresholdTask distanceThresholdTask(distanceTask.getDataPtr());
ThresholdTask tempThresholdTask(tempTask.getDataPtr());

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

    lcd.init();
    lcd.backlight();
    // LcdStdioManager::setup(&lcd);
    redirectSerialToStdio();

    printf("[Setup] Init...\n");

    redLed.init();
    greenLed.init();
    greenLed.on();
    distanceSensor.init();
    temperatureSensor.init();

    xTaskCreate(
        GenericReadingSensorTask<DistanceSensor>::taskEntryPoint,
        "Acquire",
        64,
        &distanceTask,
        3,
        NULL
    );

    xTaskCreate(
        ThresholdTask::taskEntryPoint,
        "ThreshD",
        64,
        &distanceThresholdTask,
        2,
        NULL
    );

    xTaskCreate(
        GenericReadingSensorTask<TemperatureSensor>::taskEntryPoint,
        "AcqTemp",
        64,
        &tempTask,
        3,
        NULL
    );

    xTaskCreate(
        ThresholdTask::taskEntryPoint,
        "ThreshT",
        64,
        &tempThresholdTask,
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

