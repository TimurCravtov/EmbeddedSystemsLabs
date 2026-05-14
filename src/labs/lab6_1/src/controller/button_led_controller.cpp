#include "controller/button_led_controller.h"

#include <stdio.h>

ButtonLedController::ButtonLedController(Led* led, uint16_t debounceMs, uint16_t minToggleIntervalMs)
    : debouncer_(debounceMs),
      fsm_(led),
      lastToggleMs_(0),
      minToggleIntervalMs_(minToggleIntervalMs) {}

void ButtonLedController::begin(bool initialRaw, uint32_t nowMs) {
    debouncer_.begin(initialRaw, nowMs);
    fsm_.begin();
    printf("LED: %s\n", ledStateToString(fsm_.state()));
}

void ButtonLedController::update(bool raw, uint32_t nowMs) {
    if (!debouncer_.update(raw, nowMs)) {
        return;
    }

    if (!debouncer_.stable()) {
        return;
    }

    if ((uint32_t)(nowMs - lastToggleMs_) < minToggleIntervalMs_) {
        return;
    }

    if (fsm_.dispatch(Event::ButtonPressed)) {
        printf("LED: %s\n", ledStateToString(fsm_.state()));
        lastToggleMs_ = nowMs;
    }
}
