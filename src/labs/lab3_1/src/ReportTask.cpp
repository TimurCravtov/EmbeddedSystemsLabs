#include "ReportTask.h"


void ReportTask::run(void* parameters) {
    auto* p = static_cast<ReportTaskParams*>(parameters);

    while (true) {
        bool anyAlert = false;

        for (uint8_t i = 0; i < p->sensorCount; i++) {
            SensorData* s = p->sensors[i];

            int32_t rawW;  uint32_t rawF;
            int32_t hiW;   uint32_t hiF;
            int32_t loW;   uint32_t loF;
            splitFloat(s->rawValue,      2, rawW, rawF);
            splitFloat(s->thresholdHigh, 0, hiW,  hiF);
            splitFloat(s->thresholdLow,  0, loW,  loF);

            const char* alertText;
            if (s->alertState == ALERT_ACTIVE_HIGH) alertText = "HI!";
            else if (s->alertState == ALERT_ACTIVE_LOW) alertText = "LO!";
            else if (s->alertState == ALERT_NORMAL) alertText = "OK ";
            else alertText = "PND";

            // Print exactly 16 chars per line to overwrite previous LCD content
            printf("\r%-4s:%3ld.%02lu %-3s \n", s->name, rawW, rawF, alertText);
            printf("\rT:%2ld-%2ld Db:%-3u \n", loW, hiW, s->debounceCount);

            if (s->alertActive) anyAlert = true;
        }

        if (p->onAlertUpdate) {
            p->onAlertUpdate(anyAlert);
        }

        vTaskDelay(pdMS_TO_TICKS(p->reportIntervalMs));
    }
}
