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

// LCD and Keypad configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte rowPins[4] = {9, 8, 7, 6};
byte colPins[4] = {5, 4, 3, 2};

char keys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

// Global actuator instances
// PWM LED on pin 10
PwmLed pwmLed(10);
// Servo on pin 11
ServoMotor servo(11);

void setup() {
  Serial.begin(9600);
  pwmLed.begin();
  servo.begin();

  // Set global actuator pointers for tasks
  g_pwmLed = &pwmLed;
  g_servo = &servo;

  lcd.init();
  lcd.backlight();

  static LcdStdioManager lcdManager;
  lcdManager.setup(&lcd);
  KeypadStdioManager::setup(&keypad);

  redirectSerialToStdio(false, false, true);

  // Create FreeRTOS tasks
  xTaskCreate(TaskRead, "ReadTask", 200, NULL, 1, NULL);
  xTaskCreate(TaskConditioning, "CondTask", 200, NULL, 1, NULL);
  xTaskCreate(TaskRamp, "RampTask", 200, NULL, 1, NULL);
  xTaskCreate(TaskWrite, "WriteTask", 200, NULL, 1, NULL);

  fprintf(stderr, "Servo Motor Control System\n");
  fprintf(stderr, "Keys: 0-9=0-90%%, A=100%%, B=+10%%, C=-10%%, D=50%%\n");
  fprintf(stderr, "* = Ramp mode, # = Reset\n");
  fprintf(stderr, "Setup complete. Starting scheduler...\n");
  vTaskStartScheduler();
}

void loop() {}