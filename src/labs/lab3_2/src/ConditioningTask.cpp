#include "ConditioningTask.h"
#include <semphr.h>

ConditioningTask::ConditioningTask(SignalConditioner* conditioner)
    : conditioner(conditioner), inQueue(NULL), reportEntry(NULL) {}

void ConditioningTask::setInQueue(QueueHandle_t q)    { inQueue = q; }
void ConditioningTask::setReportEntry(ReportData* r)  { reportEntry = r; }

void ConditioningTask::run() {
    while (true) {
        float raw = 0.0f;
        if (inQueue != NULL && xQueueReceive(inQueue, &raw, portMAX_DELAY) == pdTRUE) {
            conditioner->addSample(raw);

            // Update the shared report entry with the conditioned values
            if (reportEntry != NULL && reportEntry->xSemaphore != NULL) {
                xSemaphoreTake(reportEntry->xSemaphore, portMAX_DELAY);
                reportEntry->raw        = conditioner->getLastRaw();
                reportEntry->saturated  = conditioner->getLastSaturated();
                reportEntry->median     = conditioner->getLastMedian();
                reportEntry->filtered   = conditioner->getFiltered();

                // Simple status check check against min/max saturation
                if (reportEntry->raw < conditioner->getMin()) reportEntry->status = -1;
                else if (reportEntry->raw > conditioner->getMax()) reportEntry->status = 1;
                else reportEntry->status = 0;
                
                xSemaphoreGive(reportEntry->xSemaphore);
            }
        }
    }
}
