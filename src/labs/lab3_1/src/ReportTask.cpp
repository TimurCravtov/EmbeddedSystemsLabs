#include "ReportTask.h"
#include "SafePrintf.h"
#include <stdlib.h>

ReportTask::ReportTask(ReportData* data, uint8_t numberOfSensors, uint32_t delay)
    : reportData(data), numberOfSensors(numberOfSensors), delay(delay) {}

void ReportTask::run() {
    static char floatStr[10];
    while (true) {
        for (uint8_t i = 0; i < numberOfSensors; i++) {
            if (reportData[i].xSemaphore != NULL) {
                xSemaphoreTake(reportData[i].xSemaphore, portMAX_DELAY);
                int8_t s = reportData[i].status;
                float  v = reportData[i].value;
                xSemaphoreGive(reportData[i].xSemaphore);

                dtostrf(v, 4, 2, floatStr);
                safePrintf("%s :: %s :: %s\n",
                    reportData[i].sensorName,
                    s == 0 ? "OK" : (s < 0 ? "LOW" : "HIGH"),
                    floatStr);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}
