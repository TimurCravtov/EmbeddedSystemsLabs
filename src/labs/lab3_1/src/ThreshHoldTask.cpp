#include "ThreshHoldTask.h"

ThreshHoldTask::ThreshHoldTask(float minT, float maxT, uint32_t d, uint8_t debounceD, void (*callback)())
    : inQueue(NULL), minThreshold(minT), maxThreshold(maxT),
      delay(d), debounceDelay(debounceD), alertCallback(callback), reportEntry(NULL) {}

void ThreshHoldTask::setQueue(QueueHandle_t q)     { inQueue = q; }
void ThreshHoldTask::setReportEntry(ReportData* r) { reportEntry = r; }

void ThreshHoldTask::run() {
    uint8_t consecutiveBad = 0;
    uint8_t consecutiveOk = 0;
    bool alerted = false;

    while (true) {
        float value = 0.0f;
        if (inQueue != NULL && xQueueReceive(inQueue, &value, portMAX_DELAY) == pdTRUE) {
            int8_t newStatus = 0;
            if      (value < minThreshold) newStatus = -1;
            else if (value > maxThreshold) newStatus =  1;

            if (newStatus != 0) {
                // We have a threshold violation
                consecutiveOk = 0; // Reset the OK counter
                
                if (!alerted) { 
                    consecutiveBad++;
                    if (consecutiveBad >= debounceDelay) {
                        alertCallback();
                        alerted = true;
                    }
                }
            } else {
                // We have a normal reading
                consecutiveBad = 0; // Reset the bad counter
                
                if (alerted) {
                    consecutiveOk++;
                    if (consecutiveOk >= debounceDelay) {
                        alerted = false;
                        // Optional: You could add a recoveryCallback() here if needed
                    }
                }
            }

            // Note: This updates the shared report entry instantaneously with the raw reading.
            // If you want the reported status to also be debounced, you should update it 
            // inside the debounce threshold blocks above instead.
            if (reportEntry != NULL && reportEntry->xSemaphore != NULL) {
                xSemaphoreTake(reportEntry->xSemaphore, portMAX_DELAY);
                reportEntry->status = newStatus; 
                xSemaphoreGive(reportEntry->xSemaphore);
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}