#pragma once

#include <stdint.h>
#include "fsm/led_fsm.h"
#include "input/debouncer.h"

class ButtonLedController {
public:
    ButtonLedController(Led* led, uint16_t debounceMs, uint16_t minToggleIntervalMs);
    void begin(bool initialRaw, uint32_t nowMs);
    void update(bool raw, uint32_t nowMs);

private:
    Debouncer debouncer_;
    LedFsm fsm_;
    uint32_t lastToggleMs_;
    uint16_t minToggleIntervalMs_;
};
