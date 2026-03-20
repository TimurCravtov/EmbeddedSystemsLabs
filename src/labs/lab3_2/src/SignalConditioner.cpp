#include "SignalConditioner.h"

SignalConditioner::SignalConditioner(float minSat, float maxSat, uint8_t windowSize, float emaAlpha)
    : minSat(minSat), maxSat(maxSat), bufSize(windowSize),
      index(0), count(0), emaAlpha(emaAlpha), emaValue(0.0f), emaInitialised(false),
      lastRaw(0.0f), lastSaturated(0.0f), lastMedian(0.0f)
{
    buffer = new float[bufSize];
    for (uint8_t i = 0; i < bufSize; i++) buffer[i] = 0.0f;
}

float SignalConditioner::saturate(float value) const {
    if (value < minSat) return minSat;
    if (value > maxSat) return maxSat;
    return value;
}

float SignalConditioner::computeMedian() const {
    // Copy to temp array, then sort (bubble sort tiny N)
    uint8_t n = count < bufSize ? count : bufSize;
    float tmp[7]; // max window we ever use
    for (uint8_t i = 0; i < n; i++) tmp[i] = buffer[i];

    for (uint8_t i = 0; i < n - 1; i++) {
        for (uint8_t j = 0; j < n - i - 1; j++) {
            if (tmp[j] > tmp[j + 1]) {
                float t   = tmp[j];
                tmp[j]     = tmp[j + 1];
                tmp[j + 1] = t;
            }
        }
    }
    return tmp[n / 2];
}

void SignalConditioner::addSample(float raw) { 
    lastRaw = raw; 
    float sat = saturate(raw); 
    lastSaturated = sat; 
    buffer[index] = sat; 
    index = (index + 1) % bufSize; 
    if (count < bufSize) count++; 

    // 1. Compute Weighted Average
    float weightedSum = 0.0f;
    int weightSum = 0;

    for (int i = 0; i < count; i++) {
        // Calculate the actual position in the circular buffer
        // (index - 1) is the most recent sample added
        int pos = (index - 1 - i + bufSize) % bufSize;
        
        // Linear weight: newest sample gets weight 'count', oldest gets 1
        int weight = count - i; 
        weightedSum += buffer[pos] * weight;
        weightSum += weight;
    }
    
    float weightedAvg = (weightSum > 0) ? (weightedSum / (float)weightSum) : sat;

    // 2. Compute median then apply EMA (Existing Logic)
    float median = computeMedian(); 
    lastMedian = median; 
    if (!emaInitialised) { 
        emaValue = median; 
        emaInitialised = true; 
    } else { 
        emaValue = emaAlpha * median + (1.0f - emaAlpha) * emaValue; 
    } 
}

float SignalConditioner::getFiltered() const {
    return emaValue;
}

bool SignalConditioner::isReady() const {
    return count >= bufSize;
}

float SignalConditioner::getLastRaw() const       { return lastRaw; }
float SignalConditioner::getLastSaturated() const  { return lastSaturated; }
float SignalConditioner::getLastMedian() const     { return lastMedian; }
float SignalConditioner::getMin() const            { return minSat; }
float SignalConditioner::getMax() const            { return maxSat; }
