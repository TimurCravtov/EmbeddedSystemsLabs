#pragma once

#include <stdint.h>
#include <led/led.h>

enum class LedState : uint8_t { Off, On };
enum class Event : uint8_t { ButtonPressed };

class LedFsm {
public:
    explicit LedFsm(Led* led);
    void begin();
    bool dispatch(Event event);
    LedState state() const;

private:
    void apply(LedState next);

    Led* led_;
    LedState state_;
};

const char* ledStateToString(LedState state);
