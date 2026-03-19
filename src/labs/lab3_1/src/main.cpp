#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>
#include <serialio/serialio.h>
#include <math.h>
#include <semphr.h>
#include <stdlib.h>

// --- Shared print mutex (printf is not reentrant across tasks) ---
static SemaphoreHandle_t xPrintMutex = NULL;

void safePrintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (xPrintMutex != NULL) xSemaphoreTake(xPrintMutex, portMAX_DELAY);
    vprintf(fmt, args);
    fflush(stdout);   // <-- this is what was missing
    if (xPrintMutex != NULL) xSemaphoreGive(xPrintMutex);
    va_end(args);
}

class ISensor {
public:
    virtual ~ISensor() {}
    virtual float read() = 0;
};

typedef float (*Transformation)(float);
float defaultTransform(float x) { return x; }

class GenericAnalogSensor : public ISensor {
private:
    int pin;
    Transformation transform;
public:
    GenericAnalogSensor(int p, Transformation t = defaultTransform)
        : pin(p), transform(t) {
        pinMode(pin, INPUT);
    }
    float read() override {
        int raw = analogRead(pin);
        float normalized = raw / 1023.0f;
        return transform(normalized);
    }
};

class SonicDistanceSensor : public ISensor {
private:
    int trigPin;
    int echoPin;
public:
    SonicDistanceSensor(int trig, int echo) : trigPin(trig), echoPin(echo) {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }
    float read() override {
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        long duration = pulseIn(echoPin, HIGH, 30000);
        if (duration == 0) return -1.0f;
        return (duration * 0.0343f) / 2.0f;
    }
};

struct ReportData {
    int8_t status;
    const char* sensorName;
    float value;
    SemaphoreHandle_t xSemaphore;
};

class SensorAquisitionTask {
private:
    const char* taskName;
    ISensor* sensor;
    uint32_t delayMs;
    float lastValue = 0.0f;
    QueueHandle_t outQueue;
    ReportData* reportEntry;
public:
    SensorAquisitionTask(const char* name, ISensor* s, uint32_t delay)
        : taskName(name), sensor(s), delayMs(delay), outQueue(NULL), reportEntry(NULL) {}

    void setQueue(QueueHandle_t q)     { outQueue = q; }
    void setReportEntry(ReportData* r) { reportEntry = r; }

    void run() {
        while (true) {
            float value = sensor->read();
            lastValue = value;

            if (outQueue != NULL) {
                xQueueSend(outQueue, &value, 0);
            }

            if (reportEntry != NULL && reportEntry->xSemaphore != NULL) {
                xSemaphoreTake(reportEntry->xSemaphore, portMAX_DELAY);
                reportEntry->value = value;
                xSemaphoreGive(reportEntry->xSemaphore);
            }

            vTaskDelay(pdMS_TO_TICKS(delayMs));
        }
    }
    float* getLastValuePtr() { return &lastValue; }
};

class ThreshHoldTask {
private:
    QueueHandle_t inQueue;
    float minThreshold;
    float maxThreshold;
    uint32_t delay;
    uint8_t debounceDelay;
    void (*alertCallback)();
    ReportData* reportEntry;
public:
    ThreshHoldTask(float minT, float maxT, uint32_t d, uint8_t debounceD, void (*callback)())
        : inQueue(NULL), minThreshold(minT), maxThreshold(maxT),
          delay(d), debounceDelay(debounceD), alertCallback(callback), reportEntry(NULL) {}

    void setQueue(QueueHandle_t q)     { inQueue = q; }
    void setReportEntry(ReportData* r) { reportEntry = r; }

    void run() {
        uint8_t consecutiveBad = 0;
        bool alerted = false;
        while (true) {
            float value = 0.0f;
            if (inQueue != NULL && xQueueReceive(inQueue, &value, portMAX_DELAY) == pdTRUE) {
                int8_t newStatus = 0;
                if      (value < minThreshold) newStatus = -1;
                else if (value > maxThreshold) newStatus =  1;

                if (newStatus != 0) {
                    consecutiveBad++;
                    if (consecutiveBad >= debounceDelay && !alerted) {
                        alertCallback();
                        alerted = true;
                    }
                } else {
                    consecutiveBad = 0;
                    alerted = false;
                }

                if (reportEntry != NULL && reportEntry->xSemaphore != NULL) {
                    xSemaphoreTake(reportEntry->xSemaphore, portMAX_DELAY);
                    reportEntry->status = newStatus;
                    xSemaphoreGive(reportEntry->xSemaphore);
                }
            } else {
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
};

class ReportTask {
public:
    ReportTask(ReportData* data, uint8_t numberOfSensors, uint32_t delay)
        : reportData(data), numberOfSensors(numberOfSensors), delay(delay) {}

    void run() {
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
private:
    ReportData* reportData;
    uint8_t numberOfSensors;
    uint32_t delay;
};

// --- Transformations ---
float thermistorToCelsius(float normalized) {
    if (normalized <= 0.001f || normalized >= 0.999f) return 0.0f;
    const float B  = 3950.0f;
    const float R0 = 10000.0f;
    const float T0 = 298.15f;
    float resistance = 10000.0f * (normalized / (1.0f - normalized));
    float tempK = 1.0f / ((1.0f / T0) + (1.0f / B) * log(resistance / R0));
    return tempK - 273.15f;
}

// --- Static Allocations ---
GenericAnalogSensor tempSensor(A0, thermistorToCelsius);
SonicDistanceSensor distSensor(3, 2);

SensorAquisitionTask tempTaskObj("Temp (C)",  &tempSensor, 1000);
SensorAquisitionTask distTaskObj("Dist (cm)", &distSensor, 500);

void distAlertCb() { safePrintf("ALERT: Distance out of range!\n"); }
void tempAlertCb() { safePrintf("ALERT: Temperature out of range!\n"); }

ThreshHoldTask distAlertTaskObj(15.0f, 25.0f, 100, 3, distAlertCb);
ThreshHoldTask tempAlertTaskObj(10.0f, 30.0f, 100, 3, tempAlertCb);

ReportData reportDataArray[2] = {
    {0, "Temp (C)",  0.0f, NULL},
    {0, "Dist (cm)", 0.0f, NULL}
};
ReportTask reportTaskObj(reportDataArray, 2, 2000);

QueueHandle_t distQueue;
QueueHandle_t tempQueue;

void setup() {
    Serial.begin(9600);
    redirectSerialToStdio();

    xPrintMutex = xSemaphoreCreateMutex();

    distQueue = xQueueCreate(10, sizeof(float));
    tempQueue = xQueueCreate(10, sizeof(float));

    reportDataArray[0].xSemaphore = xSemaphoreCreateMutex();
    reportDataArray[1].xSemaphore = xSemaphoreCreateMutex();

    tempTaskObj.setQueue(tempQueue);
    distTaskObj.setQueue(distQueue);
    tempAlertTaskObj.setQueue(tempQueue);
    distAlertTaskObj.setQueue(distQueue);

    tempTaskObj.setReportEntry(&reportDataArray[0]);
    distTaskObj.setReportEntry(&reportDataArray[1]);
    tempAlertTaskObj.setReportEntry(&reportDataArray[0]);
    distAlertTaskObj.setReportEntry(&reportDataArray[1]);

    xTaskCreate([](void* p) { static_cast<SensorAquisitionTask*>(p)->run(); }, "TempTask",   160, &tempTaskObj,      1, NULL);
    xTaskCreate([](void* p) { static_cast<SensorAquisitionTask*>(p)->run(); }, "DistTask",   160, &distTaskObj,      1, NULL);
    xTaskCreate([](void* p) { static_cast<ThreshHoldTask*>(p)->run(); },      "AlertDist",  128, &distAlertTaskObj, 1, NULL);
    xTaskCreate([](void* p) { static_cast<ThreshHoldTask*>(p)->run(); },      "AlertTemp",  128, &tempAlertTaskObj, 1, NULL);
    xTaskCreate([](void* p) { static_cast<ReportTask*>(p)->run(); },          "ReportTask", 256, &reportTaskObj,    1, NULL);

    vTaskStartScheduler();
}

void loop() {}