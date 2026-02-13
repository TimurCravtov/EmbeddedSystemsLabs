#include <keypad/KeypadStdioManager.h>

Keypad* KeypadStdioManager::_targetKeypad = nullptr;

void KeypadStdioManager::setup(Keypad* target) {
    _targetKeypad = target;

    static FILE keypadStream;
    
    fdev_setup_stream(&keypadStream, nullptr, getChar, _FDEV_SETUP_READ);

    stdin = &keypadStream;
}

int KeypadStdioManager::getChar(FILE* stream) {
    
    if (_targetKeypad == nullptr) {
        return EOF; 
    }

    int key = NO_KEY;
    while (!(key = _targetKeypad->getKey()));
    
    return key;

}