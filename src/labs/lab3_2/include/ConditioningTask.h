#ifndef CONDITIONING_TASK_H
#define CONDITIONING_TASK_H

#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include "SignalConditioner.h"
#include "ReportData.h"

class ConditioningTask {
public:
    ConditioningTask(SignalConditioner* conditioner);

    void setInQueue(QueueHandle_t q);
    void setReportEntry(ReportData* r);
    void run();

private:
    SignalConditioner* conditioner;
    QueueHandle_t      inQueue;
    ReportData*        reportEntry;
};

#endif // CONDITIONING_TASK_H
