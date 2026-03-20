#ifndef SIGNAL_CONDITIONER_H
#define SIGNAL_CONDITIONER_H

#include <stdint.h>
#include <string.h>

class SignalConditioner {
public:
    SignalConditioner(float minSat, float maxSat, uint8_t windowSize = 5, float emaAlpha = 0.3f);

    void  addSample(float raw);
    float getFiltered() const;
    bool  isReady() const;

    // Diagnostics — last processing stages
    float getLastRaw() const;
    float getLastSaturated() const;
    float getLastMedian() const;
    float getMin() const;
    float getMax() const;

private:
    float saturate(float value) const;
    float computeMedian() const;

    float    minSat;
    float    maxSat;
    float*   buffer;
    uint8_t  bufSize;
    uint8_t  index;
    uint8_t  count;
    float    emaAlpha;
    float    emaValue;
    bool     emaInitialised;
    float    lastRaw;
    float    lastSaturated;
    float    lastMedian;
};

#endif // SIGNAL_CONDITIONER_H
