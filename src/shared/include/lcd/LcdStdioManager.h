#pragma once

#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include <stdlib.h>

class LcdStdioManager {
public:
    static void setup(LiquidCrystal_I2C* lcd);

private:
    static LiquidCrystal_I2C* _targetLcd;
    static int putChar(char c, FILE* stream);
};