#include "ReportTask.h"
#include "SafePrintf.h"
#include <stdlib.h>
#include <Arduino.h>

ReportTask::ReportTask(ReportData* data, uint8_t numberOfSensors, uint32_t delay)
    : reportData(data), numberOfSensors(numberOfSensors), delay(delay) {}

void ReportTask::run() {
    static char fRaw[10], fSat[10], fMed[10], fFil[10];
    
    while (true) {
        // --- Human-readable & Colorful Pipeline Report ---
        for (uint8_t i = 0; i < numberOfSensors; i++) {
            if (reportData[i].xSemaphore != NULL) {
                xSemaphoreTake(reportData[i].xSemaphore, portMAX_DELAY);
                
                int8_t s = reportData[i].status;
                float r = reportData[i].raw;
                float sa = reportData[i].saturated;
                float m = reportData[i].median;
                float f = reportData[i].filtered;
                float mn = reportData[i].min;
                float mx = reportData[i].max;

                xSemaphoreGive(reportData[i].xSemaphore);

                dtostrf(r, 6, 2, fRaw);
                dtostrf(sa, 6, 2, fSat);
                dtostrf(m, 6, 2, fMed);
                dtostrf(f, 6, 2, fFil);

                // --- Fancy Console Output ---
                // \x1b[32m - Green, \x1b[31m - Red, \x1b[33m - Yellow, \x1b[36m - Cyan, \x1b[0m - Reset
                
                const char* statusColor = "\x1b[32m"; // Green OK
                const char* statusStr = "OK";
                const char* barColor = "\x1b[32m";
                
                if (s < 0) { 
                    statusColor = "\x1b[33m"; // Yellow
                    statusStr = "LOW"; 
                    barColor = "\x1b[33m";
                } else if (s > 0) { 
                    statusColor = "\x1b[31m"; // Red
                    statusStr = "HIGH"; 
                    barColor = "\x1b[31m";
                }

                safePrintf("\x1b[1m[%s]\x1b[0m Status: %s%s\x1b[0m | Value: \x1b[36m%s\x1b[0m\n", 
                    reportData[i].sensorName, statusColor, statusStr, fFil);
                
                safePrintf("  Pipeline: Raw:%s -> Sat:%s -> Med:%s -> Filtered:%s\n", 
                    fRaw, fSat, fMed, fFil);

                // --- ASCII Graph ---
                int width = 20;
                float range = mx - mn;
                float percentage = (range > 0.001f) ? (f - mn) / range : 0.0f;
                if (percentage < 0.0f) percentage = 0.0f;
                if (percentage > 1.0f) percentage = 1.0f;
                int pos = (int)(percentage * width);

                safePrintf("  Graph: [");
                for (int j = 0; j < width; j++) {
                    if (j < pos) safePrintf("%s#\x1b[0m", barColor);
                    else if (j == pos) safePrintf("\x1b[33m|\x1b[0m");
                    else safePrintf("-");
                }
                safePrintf("] %d%%\n", (int)(percentage * 100));
            }
        }

        // --- Teleplot output (>label:value format) ---
        for (uint8_t i = 0; i < numberOfSensors; i++) {
            if (reportData[i].xSemaphore != NULL) {
                xSemaphoreTake(reportData[i].xSemaphore, portMAX_DELAY);
                float f = reportData[i].filtered;
                xSemaphoreGive(reportData[i].xSemaphore);

                dtostrf(f, 6, 2, fFil);
                safePrintf(">%s:%s\n", reportData[i].sensorName, fFil);
            }
        }
        
        safePrintf("--------------------------------------------------\n");

        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}
