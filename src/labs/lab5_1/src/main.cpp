#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <lcd/LcdStdioManager.h>
#include <keypad/KeypadStdioManager.h>
#include "actuator.h"
#include <serialio/serialio.h>
#include <sensors/distance.h>
#include "tasks.h"

// LCD (I2C address 0x27, 16x2)
static LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad pin configuration
static byte rowPins[4] = {9, 8, 7, 6};
static byte colPins[4] = {5, 4, 3, A1};

// Keypad layout (PROGMEM via Keypad library)
static const char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static Keypad keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

// Servo actuator on pin 11
static ServoActuator actuator(11);
// Ultrasonic sensor from nano simulation: TRIG=A0, ECHO=D2
static DistanceSensor distanceSensor(A0, 2);
static ControlContext controlContext = {&actuator, &distanceSensor};

void setup() {
    Serial.begin(9600);
    actuator.begin();
    distanceSensor.init();

    lcd.init();
    lcd.backlight();

    // STDIO: LCD output, Keypad input
    static LcdStdioManager lcdManager;
    lcdManager.setup(&lcd);
    KeypadStdioManager::setup(&keypad);

    // redirectSerialToStdio(false, false, true);

    // FreeRTOS tasks (stack sizes tuned for Nano - 2KB RAM)
    xTaskCreate(TaskRead,    "Read",   100, NULL,      1, NULL);
    xTaskCreate(TaskFilter,  "Filter", 160, &controlContext, 2, NULL);
    xTaskCreate(TaskDisplay, "Disp",   140, NULL,      1, NULL);

    // fprintf_P(stderr, PSTR("Ready\n"));
    vTaskStartScheduler();
}

void loop() {}
