#ifndef REPORT_DATA_H
#define REPORT_DATA_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdint.h>

struct ReportData {
    int8_t status;
    const char* sensorName;
    float value;
    SemaphoreHandle_t xSemaphore;
};

#endif // REPORT_DATA_H
