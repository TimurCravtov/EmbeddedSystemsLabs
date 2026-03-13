#include "ThresholdTask.h"

ThresholdTask::ThresholdTask(SensorData* sensorData) : data(sensorData) {}

void ThresholdTask::taskEntryPoint(void* parameters) {
    auto* instance = static_cast<ThresholdTask*>(parameters);
    instance->_run(parameters);
}

void ThresholdTask::_run(void* parameters) {
    while (true) {
        processThreshold();
        vTaskDelay(pdMS_TO_TICKS(data->thresholdIntervalMs));
    }
}

void ThresholdTask::processThreshold() {
    float value = data->rawValue;

    switch (data->alertState) {
    case ALERT_NORMAL:
        // Check if going above high threshold
        if (value >= data->thresholdHigh) {
            data->alertState = ALERT_PENDING_HIGH;
            data->debounceCount = 1;
        }
        // Check if going below low threshold
        else if (value <= data->thresholdLow) {
            data->alertState = ALERT_PENDING_LOW;
            data->debounceCount = 1;
        }
        break;

    case ALERT_PENDING_HIGH:
        if (value >= data->thresholdHigh) {
            if (++data->debounceCount >= data->debounceRequired) {
                data->alertState = ALERT_ACTIVE_HIGH;
                data->alertActive = true;
            }
        } else if (value <= data->thresholdLow) {
            // Switched to going low
            data->alertState = ALERT_PENDING_LOW;
            data->debounceCount = 1;
        } else {
            // Back to normal range
            data->alertState = ALERT_NORMAL;
            data->debounceCount = 0;
        }
        break;

    case ALERT_ACTIVE_HIGH:
        // Start exit when value drops back below the high threshold
        if (value < data->thresholdHigh) {
            data->alertState = ALERT_PENDING_EXIT_HIGH;
            data->debounceCount = 1;
        }
        break;

    case ALERT_PENDING_EXIT_HIGH:
        if (value < data->thresholdHigh) {
            if (++data->debounceCount >= data->debounceRequired) {
                data->alertState = ALERT_NORMAL;
                data->alertActive = false;
            }
        } else {
            // Value went back above high threshold - restore alert
            data->alertState = ALERT_ACTIVE_HIGH;
            data->debounceCount = 0;
        }
        break;

    case ALERT_PENDING_LOW:
        if (value <= data->thresholdLow) {
            if (++data->debounceCount >= data->debounceRequired) {
                data->alertState = ALERT_ACTIVE_LOW;
                data->alertActive = true;
            }
        } else if (value >= data->thresholdHigh) {
            // Switched to going high
            data->alertState = ALERT_PENDING_HIGH;
            data->debounceCount = 1;
        } else {
            // Back to normal range
            data->alertState = ALERT_NORMAL;
            data->debounceCount = 0;
        }
        break;

    case ALERT_ACTIVE_LOW:
        // Start exit when value rises back above the low threshold
        if (value > data->thresholdLow) {
            data->alertState = ALERT_PENDING_EXIT_LOW;
            data->debounceCount = 1;
        }
        break;

    case ALERT_PENDING_EXIT_LOW:
        if (value > data->thresholdLow) {
            if (++data->debounceCount >= data->debounceRequired) {
                data->alertState = ALERT_NORMAL;
                data->alertActive = false;
            }
        } else {
            // Value went back below low threshold - restore alert
            data->alertState = ALERT_ACTIVE_LOW;
            data->debounceCount = 0;
        }
        break;
    }
}
