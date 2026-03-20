#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <serialio/serialio.h>
#include <math.h>

#include "SafePrintf.h"
#include "GenericAnalogSensor.h"
#include "SonicDistanceSensor.h"
#include "SensorAquisitionTask.h"
#include "ConditioningTask.h"
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

// --- Sensors ---
GenericAnalogSensor tempSensor(A0, thermistorToCelsius);
SonicDistanceSensor distSensor(3, 2);

// --- Acquisition tasks (raw read @ configurable rate) ---
SensorAquisitionTask tempAcqTask("Temp (C)",  &tempSensor, 100);   // 100 ms
SensorAquisitionTask distAcqTask("Dist (cm)", &distSensor, 100);   // 100 ms

// --- Signal conditioners (saturation, median, weighted avg) ---
//   Temperature: clamp [-40, 125] °C, window=5, EMA alpha=0.3
SignalConditioner tempConditioner(-40.0f, 125.0f, 5, 0.3f);
//   Distance: clamp [0, 400] cm, window=5, EMA alpha=0.3
SignalConditioner distConditioner(0.0f, 400.0f, 5, 0.3f);

// --- Conditioning tasks ---
ConditioningTask tempCondTask(&tempConditioner);
ConditioningTask distCondTask(&distConditioner);

// --- Report data array (one entry per sensor) ---
ReportData reportDataArray[2] = {
    {0, "Temp (C)",  -40.0f, 125.0f, 0.0f, 0.0f, 0.0f, 0.0f, NULL},
    {0, "Dist (cm)", 0.0f,   400.0f, 0.0f, 0.0f, 0.0f, 0.0f, NULL}
};
ReportTask reportTaskObj(reportDataArray, 2, 2000);  // report every 2 s

// --- Queues ---
QueueHandle_t tempRawQueue;
QueueHandle_t distRawQueue;

void setup() {
    Serial.begin(9600);
    redirectSerialToStdio();
    safePrintfInit();

    // Create queues: acquisition → conditioning
    tempRawQueue = xQueueCreate(10, sizeof(float));
    distRawQueue = xQueueCreate(10, sizeof(float));

    // Create mutexes for report data
    reportDataArray[0].xSemaphore = xSemaphoreCreateMutex();
    reportDataArray[1].xSemaphore = xSemaphoreCreateMutex();

    // Wire acquisition tasks → raw queues (no direct report entry — conditioning writes it)
    tempAcqTask.setQueue(tempRawQueue);
    distAcqTask.setQueue(distRawQueue);

    // Wire conditioning tasks: in from raw queue, out to report entry
    tempCondTask.setInQueue(tempRawQueue);
    tempCondTask.setReportEntry(&reportDataArray[0]);

    distCondTask.setInQueue(distRawQueue);
    distCondTask.setReportEntry(&reportDataArray[1]);

    // --- Create FreeRTOS tasks ---
    // Acquisition (high priority — must not miss samples)
    xTaskCreate([](void* p) { static_cast<SensorAquisitionTask*>(p)->run(); },
                "AcqTemp", 160, &tempAcqTask, 2, NULL);
    xTaskCreate([](void* p) { static_cast<SensorAquisitionTask*>(p)->run(); },
                "AcqDist", 160, &distAcqTask, 2, NULL);

    // Conditioning (medium priority)
    xTaskCreate([](void* p) { static_cast<ConditioningTask*>(p)->run(); },
                "CondTemp", 128, &tempCondTask, 1, NULL);
    xTaskCreate([](void* p) { static_cast<ConditioningTask*>(p)->run(); },
                "CondDist", 128, &distCondTask, 1, NULL);

    // Report (lowest priority)
    xTaskCreate([](void* p) { static_cast<ReportTask*>(p)->run(); },
                "Report", 256, &reportTaskObj, 1, NULL);

    vTaskStartScheduler();
}

void loop() {}