#pragma once

#include <stdint.h>

class Debouncer {
public:
    explicit Debouncer(uint16_t debounceMs);
    void begin(bool initialRaw, uint32_t nowMs);
    bool update(bool raw, uint32_t nowMs);
    bool stable() const;

private:
    bool stable_;
    bool lastRaw_;
    uint32_t lastChangeMs_;
    uint16_t debounceMs_;
};
