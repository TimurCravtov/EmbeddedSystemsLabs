#include "Timer.h"
#include "Scheduler.h"
#include <avr/io.h>
#include <avr/interrupt.h>

namespace Timer {
    void init() {
      TCCR1A = 0;
      TCCR1B = (1 << WGM12) | (1 << CS11);
      OCR1A = (F_CPU / 8 / 1000) - 1; // the ticks withing 1 ms
      TIMSK1 = (1 << OCIE1A);
    }
}

ISR(TIMER1_COMPA_vect) { 
    Scheduler::update(); 
}

