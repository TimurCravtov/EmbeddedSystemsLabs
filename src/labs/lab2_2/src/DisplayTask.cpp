#include <stdio.h>
#include "DisplayTask.h"
#include "StatisticsTask.h"

void DisplayTask::run() {
    double average = static_cast<double>(Statistics::totalTimePressed) /
                        static_cast<double>(Statistics::shortPressesNumber);
    printf("L: %d, S: %d, Avg: %.2f\n", Statistics::longPressesNumber, Statistics::shortPressesNumber,
            average);  
}
