#pragma once

#include <led/led.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

// Password stored in program memory (defined in main.cpp)
extern const char PASSWORD[] PROGMEM;

void handleThisSupremeSecuredSystem(Led& red, Led& green, uint8_t maxPasswordLength);

byte* readPassword(uint8_t maxPasswordLength);

bool isConfiguredPasswordValid(uint8_t maxPasswordLength);
static void clearline();