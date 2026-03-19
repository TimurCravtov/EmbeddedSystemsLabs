#include "ReportTask.h"
#include <avr/pgmspace.h>

void ReportTask::run(void* parameters) {
    auto* p = static_cast<ReportTaskParams*>(parameters);

    while (true) {
        bool anyAlert = false;

        for (uint8_t i = 0; i < p->sensorCount; i++) {
            SensorData* s = p->sensors[i];

            if (xSemaphoreTake(s->xSemaphore, portMAX_DELAY) == pdTRUE) {
                // Inline the float splitting to avoid function call overhead and stack usage
                int32_t rawW = (int32_t)s->rawValue;
                uint32_t rawF = (uint32_t)(abs(s->rawValue - rawW) * 100.0f + 0.5f);
                if (rawF >= 100) { rawF = 0; rawW += (s->rawValue >= 0) ? 1 : -1; }

                const char* alertText;
                if (s->alertState == ALERT_ACTIVE_HIGH) alertText = "HI!";
                else if (s->alertState == ALERT_ACTIVE_LOW) alertText = "LO!";
                else if (s->alertState == ALERT_NORMAL) alertText = "OK ";
                else alertText = "PND";

                // Print exactly 16 chars per line to overwrite previous LCD content
                // simplified thresholds to basic integers to relieve all buffer/stack pressure
                printf_P(PSTR("\r%-4s:%3ld.%02lu %-3s \n"), s->name, rawW, rawF, alertText);
                printf_P(PSTR("\rT:%2d-%2d Db:%-3u \n"), (int)s->thresholdLow, (int)s->thresholdHigh, s->debounceCount);

                if (s->alertActive) anyAlert = true;

                xSemaphoreGive(s->xSemaphore);
            }
        }

        if (p->onAlertUpdate) {
            p->onAlertUpdate(anyAlert);
        }

        vTaskDelay(pdMS_TO_TICKS(p->reportIntervalMs));
    }
}
