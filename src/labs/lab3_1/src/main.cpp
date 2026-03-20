#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <serialio/serialio.h>
#include <math.h>

#include "SafePrintf.h"
#include "GenericAnalogSensor.h"
#include "SonicDistanceSensor.h"
#include "SensorAquisitionTask.h"
#include "ThreshHoldTask.h"
#include "ReportTask.h"

// --- Transformations ---
float thermistorToCelsius(float normalized) {
    if (normalized <= 0.001f || normalized >= 0.999f) return 0.0f;
    const float B  = 3950.0f;
    const float R0 = 10000.0f;
    const float T0 = 298.15f;
    float resistance = 10000.0f * (normalized / (1.0f - normalized));
    float tempK = 1.0f / ((1.0f / T0) + (1.0f / B) * log(resistance / R0));
    return tempK - 273.15f;
}

// --- Static Allocations ---
GenericAnalogSensor tempSensor(A0, thermistorToCelsius);
SonicDistanceSensor distSensor(3, 2);

SensorAquisitionTask tempTaskObj("Temp (C)",  &tempSensor, 1000);
SensorAquisitionTask distTaskObj("Dist (cm)", &distSensor, 500);

void distAlertCb() { safePrintf("ALERT: Distance out of range!\n"); }
void tempAlertCb() { safePrintf("ALERT: Temperature out of range!\n"); }

ThreshHoldTask distAlertTaskObj(15.0f, 25.0f, 100, 3, distAlertCb);
ThreshHoldTask tempAlertTaskObj(10.0f, 30.0f, 100, 3, tempAlertCb);

ReportData reportDataArray[2] = {
    {0, "Temp (C)",  0.0f, NULL},
    {0, "Dist (cm)", 0.0f, NULL}
};
ReportTask reportTaskObj(reportDataArray, 2, 500);

QueueHandle_t distQueue;
QueueHandle_t tempQueue;

void setup() {
    Serial.begin(9600);
    redirectSerialToStdio();

    safePrintfInit();

    distQueue = xQueueCreate(10, sizeof(float));
    tempQueue = xQueueCreate(10, sizeof(float));

    reportDataArray[0].xSemaphore = xSemaphoreCreateMutex();
    reportDataArray[1].xSemaphore = xSemaphoreCreateMutex();

    tempTaskObj.setQueue(tempQueue);
    distTaskObj.setQueue(distQueue);
    tempAlertTaskObj.setQueue(tempQueue);
    distAlertTaskObj.setQueue(distQueue);

    tempTaskObj.setReportEntry(&reportDataArray[0]);
    distTaskObj.setReportEntry(&reportDataArray[1]);
    tempAlertTaskObj.setReportEntry(&reportDataArray[0]);
    distAlertTaskObj.setReportEntry(&reportDataArray[1]);

    xTaskCreate([](void* p) { static_cast<SensorAquisitionTask*>(p)->run(); }, "TempTask",   160, &tempTaskObj,      1, NULL);
    xTaskCreate([](void* p) { static_cast<SensorAquisitionTask*>(p)->run(); }, "DistTask",   160, &distTaskObj,      1, NULL);
    xTaskCreate([](void* p) { static_cast<ThreshHoldTask*>(p)->run(); },      "AlertDist",  128, &distAlertTaskObj, 1, NULL);
    xTaskCreate([](void* p) { static_cast<ThreshHoldTask*>(p)->run(); },      "AlertTemp",  128, &tempAlertTaskObj, 1, NULL);
    xTaskCreate([](void* p) { static_cast<ReportTask*>(p)->run(); },          "ReportTask", 256, &reportTaskObj,    1, NULL);

    vTaskStartScheduler();
}

void loop() {}