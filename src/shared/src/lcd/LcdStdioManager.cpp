#include <lcd/LcdStdioManager.h>

LiquidCrystal_I2C* LcdStdioManager::_targetLcd = nullptr;
static uint8_t currentRow = 0;
static uint8_t currentCol = 0;

void LcdStdioManager::setup(LiquidCrystal_I2C* lcd) {
    _targetLcd = lcd;
    static FILE lcdout;
    fdev_setup_stream(&lcdout, putChar, NULL, _FDEV_SETUP_WRITE);
    stdout = &lcdout;
}

// remember: setCursor(x,y) means x columns and y rows
int LcdStdioManager::putChar(char c, FILE* stream) {
    if (_targetLcd) {

        // that's peak performance
        if (c == '\n') {
            
            currentRow = (currentRow + 1) % 2;  
            currentCol = 0;
            _targetLcd->setCursor(currentCol, currentRow);

        } else if (c == '\r') {
            currentCol = 0;
            _targetLcd->setCursor(0, currentRow);
        } else {
            _targetLcd->write(c);
            currentCol++;

        }
    }
    return 0;
}
