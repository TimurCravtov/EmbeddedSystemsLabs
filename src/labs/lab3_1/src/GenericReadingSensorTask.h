#pragma once

#include "GenericTask.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Arduino.h>

enum AlertState : uint8_t {
    ALERT_NORMAL,         // within threshold band
    ALERT_PENDING_HIGH,   // debouncing to enter HIGH alert
    ALERT_ACTIVE_HIGH,    // above high threshold, alert confirmed
    ALERT_PENDING_EXIT_HIGH, // debouncing to exit HIGH alert
    ALERT_PENDING_LOW,    // debouncing to enter LOW alert
    ALERT_ACTIVE_LOW,     // below low threshold, alert confirmed
    ALERT_PENDING_EXIT_LOW    // debouncing to exit LOW alert
};

struct SensorConfig {
    uint16_t readingIntervalMs;  // acquisition recurrence (20-100 ms)
    uint16_t thresholdIntervalMs; // conditioning recurrence
    float thresholdCenter;       // center threshold value
    float hysteresis;            // +- band around center
    uint8_t debounceRequired;    // consecutive readings to confirm state change
    const char* name;            // sensor label for reporting
};

struct SensorData {
    volatile float rawValue;
    float thresholdHigh;
    float thresholdLow;
    volatile AlertState alertState;
    volatile uint8_t debounceCount;
    volatile bool alertActive;  // true if ALERT_ACTIVE_HIGH or ALERT_ACTIVE_LOW
    const char* name;
    uint16_t thresholdIntervalMs;
    uint8_t debounceRequired;
    SemaphoreHandle_t xSemaphore;
};

// Task 1: Acquisition just reads raw sensor data periodically
// SensorType must provide: float readRaw()
template <typename SensorType>
class GenericReadingSensorTask : public GenericTask<GenericReadingSensorTask<SensorType>> {
private:
    SensorType& sensor;
    SensorConfig config;
    SensorData data;
    float (*converter)(float);

public:
    GenericReadingSensorTask(SensorType& sensor, const SensorConfig& cfg, float (*conv)(float) = nullptr)
        : sensor(sensor), config(cfg), converter(conv)
    {
        data.rawValue            = 0;
        data.thresholdHigh       = cfg.thresholdCenter + cfg.hysteresis;
        data.thresholdLow        = cfg.thresholdCenter - cfg.hysteresis;
        data.alertState          = ALERT_NORMAL;
        data.debounceCount       = 0;
        data.alertActive         = false;
        data.name                = cfg.name;
        data.thresholdIntervalMs = cfg.thresholdIntervalMs;
        data.debounceRequired    = cfg.debounceRequired;
        data.xSemaphore          = NULL;
    }

    void init() {
        if (data.xSemaphore == NULL) {
            data.xSemaphore = xSemaphoreCreateMutex();
        }
    }

    SensorData* getDataPtr() { return &data; }

    static void taskEntryPoint(void* parameters) {
        auto* instance = static_cast<GenericReadingSensorTask*>(parameters);
        instance->_run(parameters);
    }

    void _run(void* parameters) {
        while (true) {
            float val = sensor.readRaw();
            if (converter != nullptr) {
                val = converter(val);
            }

            if (xSemaphoreTake(data.xSemaphore, portMAX_DELAY) == pdTRUE) {
                data.rawValue = val;
                xSemaphoreGive(data.xSemaphore);
            }

            vTaskDelay(pdMS_TO_TICKS(config.readingIntervalMs));
        }
    }
};
