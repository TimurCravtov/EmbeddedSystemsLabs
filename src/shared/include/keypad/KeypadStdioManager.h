#pragma once

#include <Keypad.h>

class KeypadStdioManager {
public:
    static void setup(Keypad* target);

private:
    static Keypad* _targetKeypad;
    static int getChar(FILE* stream);
};