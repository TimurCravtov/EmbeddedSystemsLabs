#include <Arduino.h>

#include <stdio.h>
#include <stdlib.h>
#include <led/led.h> 
#include <serialio/serialio.h>
#include <string.h>
#include <LedController.h>

constexpr uint8_t ledPinNum = 13;
Led led(ledPinNum);

void setup() {

  led.init();
  // start serial communication
  Serial.begin(9600);
  delay(1000);
  
  redirectSerialToStdio();

}

void loop() {

    // reading command from serial
    char buffer[7] = {0};
      
    // promt to read data
    printf_P(PSTR("Waiting for command (led on / led off): \n> "));

    // read the line
    scanf(" %[^\n\r]", buffer);
    int c;

    // clear the buffer
    while ((c = getchar()) != '\n' && c != '\r' && c != EOF);

    // process command introduced by user
    processCommand(led, buffer);
  
}
