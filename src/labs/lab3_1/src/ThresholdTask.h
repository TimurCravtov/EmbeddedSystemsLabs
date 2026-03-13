#pragma once

#include "GenericReadingSensorTask.h"
#include "GenericTask.h"
#include <Arduino_FreeRTOS.h>

class ThresholdTask : public GenericTask<ThresholdTask> {
private:
    SensorData* data;
    void processThreshold();

public:
    ThresholdTask(SensorData* sensorData);
    static void taskEntryPoint(void* parameters);
    void _run(void* parameters);
};
