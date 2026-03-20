#ifndef SENSOR_AQUISITION_TASK_H
#define SENSOR_AQUISITION_TASK_H

#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include "ISensor.h"
#include "ReportData.h"

class SensorAquisitionTask {
private:
    const char* taskName;
    ISensor* sensor;
    uint32_t delayMs;
    float lastValue;
    QueueHandle_t outQueue;
    ReportData* reportEntry;
public:
    SensorAquisitionTask(const char* name, ISensor* s, uint32_t delay);

    void setQueue(QueueHandle_t q);
    void setReportEntry(ReportData* r);
    void run();
    float* getLastValuePtr();
};

#endif // SENSOR_AQUISITION_TASK_H
