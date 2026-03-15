#pragma once

#include "GenericReadingSensorTask.h"
#include <serialio/serialio.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>

struct ReportTaskParams {
    SensorData** sensors;
    uint8_t sensorCount;
    uint16_t reportIntervalMs;
    void (*onAlertUpdate)(bool anyAlert);
};



namespace ReportTask {
    void run(void* parameters);
}
