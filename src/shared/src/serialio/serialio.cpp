#include <Arduino.h>
#include <serialio/serialio.h>
#include <stdio.h>

#include <stdlib.h>

#include <stdint.h>
#include <math.h>

// add a char to serial output
int serialPutchar(char c, FILE* stream) {
  if (c == '\n') {
    Serial.write('\r');
  }
  Serial.write(c);
  return 0;
}

// get a char from serial input
int serialGetchar(FILE* stream) {
  while (!Serial.available());
  return Serial.read();
}

// Helper functions for redirecting Serial to stdio
void redirectSerialToStdio(bool in = true, bool out = true, bool err = true) {
  static FILE uartinout;

  fdev_setup_stream(&uartinout, serialPutchar, serialGetchar, _FDEV_SETUP_RW);

  if (out) stdout = &uartinout;
  if (in) stdin = &uartinout;
  if (err) stderr = &uartinout;
}


// Utility to split a float into whole and fractional integer parts
void splitFloat(float value, uint8_t decimalPlaces, int32_t& wholePart, uint32_t& fracPart) {

    wholePart = (int32_t)value;
    
    float remainder = value - (float)wholePart;
    if (remainder < 0.0f) {
        remainder = -remainder;
    }
    
    uint32_t multiplier = 1;
    for (uint8_t i = 0; i < decimalPlaces; i++) {
        multiplier *= 10;
    }
    
    fracPart = (uint32_t)(remainder * multiplier + 0.5f);
    
    // 5. Handle edge case: if rounding pushes the fraction up to a full whole number
    if (fracPart >= multiplier) {
        fracPart = 0;
        if (value >= 0.0f) {
            wholePart += 1;
        } else {
            wholePart -= 1;
        }
    }
}

char* colorCode(ConsoleColor color) {
    switch (color) {
        case ConsoleColor::BLACK:         return "\033[30m";
        case ConsoleColor::RED:           return "\033[31m";
        case ConsoleColor::GREEN:         return "\033[32m";
        case ConsoleColor::YELLOW:        return "\033[33m";
        case ConsoleColor::BLUE:          return "\033[34m";
        case ConsoleColor::MAGENTA:       return "\033[35m";
        case ConsoleColor::CYAN:          return "\033[36m";
        case ConsoleColor::WHITE:         return "\033[37m";
        case ConsoleColor::BRIGHT_BLACK:  return "\033[90m";
        case ConsoleColor::BRIGHT_RED:    return "\033[91m";
        case ConsoleColor::BRIGHT_GREEN:  return "\033[92m";
        case ConsoleColor::BRIGHT_YELLOW: return "\033[93m";
        case ConsoleColor::BRIGHT_BLUE:   return "\033[94m";
        case ConsoleColor::BRIGHT_MAGENTA:return "\033[95m";
        case ConsoleColor::BRIGHT_CYAN:   return "\033[96m";
        case ConsoleColor::BRIGHT_WHITE:  return "\033[97m";
        default:                         return "";
    }
}