#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <lcd/LcdStdioManager.h>
#include <keypad/KeypadStdioManager.h>
#include <Keypad.h>
#include <led/led.h>
#include "PinCodeSystem.h"
// #include <serialio/serialio.h>

// ketpadd stuff
constexpr byte ROWS = 4; 
constexpr byte COLS = 4; 

// Keymap must be in RAM: Keypad library reads it with keymap[i], not pgm_read_byte()
const char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

constexpr byte rowPins[ROWS] = {9, 8, 7, 6}; 
constexpr byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), const_cast<byte*>(rowPins), const_cast<byte*>(colPins), ROWS, COLS);


// lcd
constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLS = 16;
constexpr uint8_t LCD_ROWS = 2;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// leds
constexpr uint8_t redLedPin = 12;
Led redLed(redLedPin);

constexpr uint8_t greenLedPin = 13;
Led greenLed(greenLedPin);

// PASSWORD FOR ACTIVATING THE SYSTEM (must be numeric and not exceed maxPasswordLength)
constexpr uint8_t maxPasswordLength = 10;
const char PASSWORD[] PROGMEM = "123653";

void setup() {
    lcd.init();
    
    if (!isConfiguredPasswordValid(maxPasswordLength)) {
        printf_P(PSTR("Invalid password\n"));
        while (true) {
        delay(1000);
        }
    }
    // Serial.begin(9600);

    lcd.backlight();
    redLed.init(); 

    // redirectErrorToSerial();
    greenLed.init();
    LcdStdioManager::setup(&lcd);
    KeypadStdioManager::setup(&keypad);
    // fprintf_P(stderr, PSTR("System initialized\n\rTest"));
}

void loop() {

    handleThisSupremeSecuredSystem(redLed, greenLed, maxPasswordLength);
}

 
