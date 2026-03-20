#include "SensorAquisitionTask.h"

SensorAquisitionTask::SensorAquisitionTask(const char* name, ISensor* s, uint32_t delay)
    : taskName(name), sensor(s), delayMs(delay), lastValue(0.0f),
      outQueue(NULL), reportEntry(NULL) {}

void SensorAquisitionTask::setQueue(QueueHandle_t q)     { outQueue = q; }
void SensorAquisitionTask::setReportEntry(ReportData* r) { reportEntry = r; }

void SensorAquisitionTask::run() {
    while (true) {
        float value = sensor->read();
        lastValue = value;

        if (outQueue != NULL) {
            xQueueSend(outQueue, &value, 0);
        }

        if (reportEntry != NULL && reportEntry->xSemaphore != NULL) {
            xSemaphoreTake(reportEntry->xSemaphore, portMAX_DELAY);
            reportEntry->raw = value;
            xSemaphoreGive(reportEntry->xSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}

float* SensorAquisitionTask::getLastValuePtr() { return &lastValue; }
