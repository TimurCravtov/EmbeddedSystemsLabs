#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <lcd/LcdStdioManager.h>
#include <keypad/KeypadStdioManager.h>
#include "actuator.h"
#include <serialio/serialio.h>
#include "tasks.h"

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad configuration
byte rowPins[4] = {9, 8, 7, 6};
byte colPins[4] = {5, 4, 3, 2};

char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

// Global actuator instance (PWM on pin 10)
PwmActuator actuator(10);

void setup() {
    Serial.begin(9600);
    actuator.begin();

    lcd.init();
    lcd.backlight();

    // Static prevents object destruction after setup ends
    static LcdStdioManager lcdManager;
    lcdManager.setup(&lcd);
    KeypadStdioManager::setup(&keypad);

    redirectSerialToStdio(false, false, true);

    // Create FreeRTOS tasks
    xTaskCreate(TaskRead,    "ReadTask",    200, NULL,         1, NULL);
    xTaskCreate(TaskFilter,  "FilterTask",  200, &actuator,    2, NULL);
    xTaskCreate(TaskDisplay, "DisplayTask", 200, NULL,         1, NULL);

    fprintf(stderr, "Setup complete. Starting scheduler...\n");
    vTaskStartScheduler();
}

void loop() {}
