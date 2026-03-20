#include "SafePrintf.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <stdarg.h>

static SemaphoreHandle_t xPrintMutex = NULL;

void safePrintfInit() {
    xPrintMutex = xSemaphoreCreateMutex();
}

void safePrintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (xPrintMutex != NULL) xSemaphoreTake(xPrintMutex, portMAX_DELAY);
    vprintf(fmt, args);
    fflush(stdout);
    if (xPrintMutex != NULL) xSemaphoreGive(xPrintMutex);
    va_end(args);
}
