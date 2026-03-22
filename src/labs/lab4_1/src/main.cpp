#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <serialio/serialio.h>

#include <Keypad.h>
#include <Wire.h>

#include <lcd/LcdStdioManager.h>
#include <button/button.h>
#include <keypad/KeypadStdioManager.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte rowPins[4] = {9, 8, 7, 6}; 
byte colPins[4] = {5, 4, 3, 2}; 
char keys[4][4] = {
  {'+','-','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

class BinaryActuator {
    virtual void on() = 0;
    virtual void off() = 0;
    virtual bool isOn() = 0;
};

void TaskRead(void* pvParameters) {
    while(true) {
        char c;
        // This will now be non-blocking based on the fixed KeypadStdioManager
        if (scanf("%c", &c) != EOF) {
            // Process character
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}

void setup() {
    Serial.begin(9600);

    // Initialize LCD
    lcd.init();
    lcd.backlight();

    // Setup Managers
    LcdStdioManager lcdManager;
    lcdManager.setup(&lcd); // Redirects stdout to LCD

    KeypadStdioManager::setup(&keypad); // Redirects stdin to Keypad

    // Start Tasks
    xTaskCreate(TaskRead, "ReadTask", 128, NULL, 1, NULL);
}

void loop() { }