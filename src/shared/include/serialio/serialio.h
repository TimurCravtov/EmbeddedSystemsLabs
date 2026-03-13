#pragma once

#include <Arduino.h>

// this function does exactly what do you think it does
void redirectSerialToStdio(bool in = true, bool out = true, bool err = true);
void redirectErrorToSerial();
void splitFloat(float value, uint8_t decimalPlaces, int32_t& wholePart, uint32_t& fracPart);

enum class ConsoleColor {
    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
    BRIGHT_BLACK, BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW,
    BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
};

char* colorCode(ConsoleColor color);