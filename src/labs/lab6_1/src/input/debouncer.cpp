#include "input/debouncer.h"

Debouncer::Debouncer(uint16_t debounceMs)
    : stable_(false), lastRaw_(false), lastChangeMs_(0), debounceMs_(debounceMs) {}

void Debouncer::begin(bool initialRaw, uint32_t nowMs) {
    stable_ = initialRaw;
    lastRaw_ = initialRaw;
    lastChangeMs_ = nowMs;
}

bool Debouncer::update(bool raw, uint32_t nowMs) {
    if (raw != lastRaw_) {
        lastRaw_ = raw;
        lastChangeMs_ = nowMs;
    }

    if ((uint32_t)(nowMs - lastChangeMs_) >= debounceMs_ && raw != stable_) {
        stable_ = raw;
        return true;
    }

    return false;
}

bool Debouncer::stable() const {
    return stable_;
}
