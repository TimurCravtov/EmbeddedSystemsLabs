#pragma once

#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include "ReportData.h"

class ThreshHoldTask {
private:
    QueueHandle_t inQueue;
    float minThreshold;
    float maxThreshold;
    uint32_t delay;
    uint8_t debounceDelay;
    void (*alertCallback)();
    ReportData* reportEntry;
public:
    ThreshHoldTask(float minT, float maxT, uint32_t d, uint8_t debounceD, void (*callback)());

    void setQueue(QueueHandle_t q);
    void setReportEntry(ReportData* r);
    void run();
};
