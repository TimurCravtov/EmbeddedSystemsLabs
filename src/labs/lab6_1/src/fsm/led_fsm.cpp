#include "fsm/led_fsm.h"

struct Transition {
    LedState from;
    Event event;
    LedState to;
};

static const Transition FSM_TRANSITIONS[] = {
    {LedState::Off, Event::ButtonPressed, LedState::On},
    {LedState::On,  Event::ButtonPressed, LedState::Off}
};

LedFsm::LedFsm(Led* led) : led_(led), state_(LedState::Off) {}

void LedFsm::begin() {
    apply(state_);
}

bool LedFsm::dispatch(Event event) {
    const LedState prev = state_;

    for (const Transition& t : FSM_TRANSITIONS) {
        if (t.from == state_ && t.event == event) {
            apply(t.to);
            break;
        }
    }

    return state_ != prev;
}

LedState LedFsm::state() const {
    return state_;
}

void LedFsm::apply(LedState next) {
    state_ = next;

    if (!led_) {
        return;
    }

    if (state_ == LedState::On) {
        led_->on();
    } else {
        led_->off();
    }
}

const char* ledStateToString(LedState state) {
    return (state == LedState::On) ? "ON" : "OFF";
}
