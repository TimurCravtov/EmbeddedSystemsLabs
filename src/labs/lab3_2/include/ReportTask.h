#ifndef REPORT_TASK_H
#define REPORT_TASK_H

#include <Arduino_FreeRTOS.h>
#include "ReportData.h"

class ReportTask {
public:
    ReportTask(ReportData* data, uint8_t numberOfSensors, uint32_t delay);
    void run();
private:
    ReportData* reportData;
    uint8_t numberOfSensors;
    uint32_t delay;
};

#endif // REPORT_TASK_H
