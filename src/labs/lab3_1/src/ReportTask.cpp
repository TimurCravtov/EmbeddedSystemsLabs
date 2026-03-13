#include "ReportTask.h"

const char* alertStateStr(AlertState s) {
    switch (s) {
        case ALERT_NORMAL:            return "NORMAL";
        case ALERT_PENDING_HIGH:      return "PEND_HI";
        case ALERT_ACTIVE_HIGH:       return "HIGH!";
        case ALERT_PENDING_EXIT_HIGH: return "EXIT_HI";
        case ALERT_PENDING_LOW:       return "PEND_LO";
        case ALERT_ACTIVE_LOW:        return "LOW!";
        case ALERT_PENDING_EXIT_LOW:  return "EXIT_LO";
        default:                      return "?";
    }
}

ConsoleColor alertStateColor(AlertState s) {
    switch (s) {
        case ALERT_NORMAL:            return ConsoleColor::GREEN;
        case ALERT_PENDING_HIGH:      return ConsoleColor::YELLOW;
        case ALERT_ACTIVE_HIGH:       return ConsoleColor::RED;
        case ALERT_PENDING_EXIT_HIGH: return ConsoleColor::YELLOW;
        case ALERT_PENDING_LOW:       return ConsoleColor::YELLOW;
        case ALERT_ACTIVE_LOW:        return ConsoleColor::MAGENTA;  // or CYAN for low alert
        case ALERT_PENDING_EXIT_LOW:  return ConsoleColor::YELLOW;
        default:                      return ConsoleColor::WHITE;
    }
}

void ReportTask::run(void* parameters) {
    auto* p = static_cast<ReportTaskParams*>(parameters);

    while (true) {
        bool anyAlert = false;

        printf("====== Report ====== \n");
        for (uint8_t i = 0; i < p->sensorCount; i++) {
            SensorData* s = p->sensors[i];

            int32_t rawW;  uint32_t rawF;
            int32_t hiW;   uint32_t hiF;
            int32_t loW;   uint32_t loF;
            splitFloat(s->rawValue,      2, rawW, rawF);
            splitFloat(s->thresholdHigh, 1, hiW,  hiF);
            splitFloat(s->thresholdLow,  1, loW,  loF);

            printf("[%s] Val:%ld.%02lu\n", s->name, rawW, rawF);
            printf(" Thr:%ld.%lu-%ld.%lu\n", loW, loF, hiW, hiF);

            char* clr = colorCode(alertStateColor(s->alertState));
            char* rst = colorCode(ConsoleColor::WHITE);
            printf(" St:%s%s%s Db:%u\n", clr, alertStateStr(s->alertState), rst, s->debounceCount);

            // Show alert direction
            const char* alertText;
            char* alertClr;
            if (s->alertState == ALERT_ACTIVE_HIGH) {
                alertText = "HIGH!";
                alertClr = colorCode(ConsoleColor::BRIGHT_RED);
            } else if (s->alertState == ALERT_ACTIVE_LOW) {
                alertText = "LOW!";
                alertClr = colorCode(ConsoleColor::BRIGHT_MAGENTA);
            } else {
                alertText = "OK";
                alertClr = colorCode(ConsoleColor::BRIGHT_GREEN);
            }
            printf(" Alert:%s%s%s\n", alertClr, alertText, rst);

            if (s->alertActive) anyAlert = true;
        }
        printf("==============\n");

        if (p->onAlertUpdate) {
            p->onAlertUpdate(anyAlert);
        }

        vTaskDelay(pdMS_TO_TICKS(p->reportIntervalMs));
    }
}
