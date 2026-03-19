#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <stdio.h>
#include <serialio/serialio.h>
#include <math.h>

#include <semphr.h>

// --- Interface & Types ---
class ISensor {
public:
    virtual ~ISensor() {}
    virtual float read() = 0; 
};

typedef float (*Transformation)(float);
float defaultTransform(float x) { return x; }

// --- Analog Sensor Implementation ---
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

// --- Digital Sensor Implementation (HC-SR04) ---
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


float saturate(float value, float minVal, float maxVal) {
    return value < minVal ? minVal : (value > maxVal ? maxVal : value);
}

float medianFilter(float* buffer, uint8_t bufferSize) {
    float sorted[bufferSize];
    memcpy(sorted, buffer, bufferSize * sizeof(float));
    for (uint8_t i = 0; i < bufferSize - 1; i++) {
        for (uint8_t j = 0; j < bufferSize - i - 1; j++) {
            if (sorted[j] > sorted[j + 1]) {
                float temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    return sorted[bufferSize / 2];
}

float movingAverage(float* buffer, uint8_t bufferSize) {
    float sum = 0.0f;
    for (uint8_t i = 0; i < bufferSize; i++) {
        sum += buffer[i];
    }
    return sum / bufferSize;
}

class SignalConditioner {
public:
    SignalConditioner(float minSaturate, float maxSaturate) : minSaturate(minSaturate), maxSaturate(maxSaturate) {
        bufferSize = 5; 
        buffer = new float[bufferSize];
    }

    void addSample(float sample) {
        buffer[index] = saturate(sample, minSaturate, maxSaturate);
        index = (index + 1) % bufferSize;
        if (count < bufferSize) count++;
    }

private:
    float minSaturate;
    float maxSaturate;
    uint8_t bufferSize;
    float* buffer;
    uint8_t index = 0;
    uint8_t count = 0;
};

class SensorAquisitionTask {
private:
    const char* taskName;
    ISensor* sensor;
    uint32_t delayMs;
    float lastValue = 0.0f;
    QueueHandle_t outQueue = NULL;
public:

    SensorAquisitionTask(const char* name, ISensor* s, uint32_t delay, QueueHandle_t q = NULL) 
        : taskName(name), sensor(s), delayMs(delay), outQueue(q) {}
    
    void run() {
        while (true) {
            float value = sensor->read();
            lastValue = value;
            printf("%s: %d.%02d\n", taskName, (int)value, (int)(abs(value) * 100) % 100);
            if (outQueue != NULL) {
                xQueueSend(outQueue, &value, 0);
            }
            vTaskDelay(pdMS_TO_TICKS(delayMs)); 
        }
    }
    float* getLastValuePtr() { return &lastValue; }
};


struct ReportData {
    uint8_t status; // -1 low, 0 normal, 1 high
    char* sensorName;
    float value;
    SemaphoreHandle_t xSemaphore; // read write semaphore
};

class ReportTask {
public:
    ReportTask(ReportData* data, uint8_t numberOfSensors, uint32_t delay) : reportData(data), numberOfSensors(numberOfSensors), delay(delay) {}

    void run() {
        while (true) {
            for (uint8_t i = 0; i < this->numberOfSensors; i++) {
                xSemaphoreTake(reportData[i].xSemaphore, portMAX_DELAY);
                printf("%s :: %s :: %s\n", reportData[i].sensorName, 
                    reportData[i].status == 0 ? "OK" : (reportData[i].status < 0 ? "LOW" : "HIGH"), 
                    String(reportData[i].value).c_str());
                xSemaphoreGive(reportData[i].xSemaphore);
            }
            vTaskDelay(pdMS_TO_TICKS(this->delay));
        }
    }
private:
    ReportData* reportData;
    uint8_t numberOfSensors;
    uint32_t delay;
};

// --- Conversion Logic ---
float thermistorToCelsius(float normalized) {
    if (normalized <= 0.001f || normalized >= 0.999f) return 0.0f;
    const float B = 3950.0f;          
    const float R0 = 10000.0f;        
    const float T0 = 298.15f;         
    float resistance = 10000.0f * (normalized / (1.0f - normalized));
    float tempK = 1.0f / ((1.0f / T0) + (1.0f / B) * log(resistance / R0));
    return tempK - 273.15f; 
}



// --- Main Setup ---
void setup() {
    Serial.begin(9600);
    redirectSerialToStdio();

    ISensor* temp = new GenericAnalogSensor(A0, thermistorToCelsius);
    ISensor* distance = new SonicDistanceSensor(3, 2); 
    
    auto* tempTask = new SensorAquisitionTask("Temp (C)", temp, 1000);
    xTaskCreate([](void* p) {
        static_cast<SensorAquisitionTask*>(p)->run();
    }, "TempTask", 128, tempTask, 1, NULL);

    // Create queue for distance samples (10-sample buffer)
    QueueHandle_t distQueue = xQueueCreate(10, sizeof(float));

    auto* distTask = new SensorAquisitionTask("Dist (cm)", distance, 500, distQueue);
    xTaskCreate([](void* p) {
        static_cast<SensorAquisitionTask*>(p)->run();
    }, "DistTask", 128, distTask, 1, NULL);

    // Create threshold task that consumes from the distance queue (5-sample debounce)
    xTaskCreate([](void* p) {
        static_cast<ThreshHoldTask*>(p)->run();
    }, "AlertTask", 128, new ThreshHoldTask(distQueue, 15.0f, 25.0f, 100, 3, []() {
        printf("ALERT: Distance out of range!\n");
    }), 1, NULL);

    vTaskStartScheduler();
}

void loop() {}