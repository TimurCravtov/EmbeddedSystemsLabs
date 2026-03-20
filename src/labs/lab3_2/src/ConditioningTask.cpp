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
            float filtered = conditioner->getFiltered();

            // Update the shared report entry with the conditioned value
            if (reportEntry != NULL && reportEntry->xSemaphore != NULL) {
                xSemaphoreTake(reportEntry->xSemaphore, portMAX_DELAY);
                reportEntry->value = filtered;

                // Simple status check — could be made configurable
                reportEntry->status = 0; // conditioner doesn't judge thresholds
                xSemaphoreGive(reportEntry->xSemaphore);
            }
        }
    }
}
