#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <lcd/LcdStdioManager.h>
#include <keypad/KeypadStdioManager.h>
#include "actuator.h"
#include "tasks.h"

// LCD and Keypad configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte rowPins[4] = {9, 8, 7, 6};
byte colPins[4] = {5, 4, 3, 2};

char keys[4][4] = {
  {'0','1','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

// Global actuator instance
RelayActuator relay(10);

void setup() {
    Serial.begin(9600);
    relay.begin();

    lcd.init();
    lcd.backlight();

    // static prevents object destruction after setup ends
    static LcdStdioManager lcdManager;
    lcdManager.setup(&lcd);
    KeypadStdioManager::setup(&keypad);

    // Create FreeRTOS tasks
    xTaskCreate(TaskRead, "ReadTask", 128, NULL, 1, NULL);
    xTaskCreate(TaskActuatorConditioning, "CondTask", 128, NULL, 1, NULL);
    xTaskCreate(TaskWrite, "WriteTask", 128, &relay, 1, NULL);
}

void loop() { }