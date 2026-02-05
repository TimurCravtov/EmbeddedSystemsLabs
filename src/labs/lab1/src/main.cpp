#include <Arduino.h>

#include <stdio.h>
#include <stdlib.h>
#include <led/led.h> 
#include <serialio/serialio.h>
#include <string.h>
#include <LedController.h>

const uint8_t ledPinNum = 3 ;

Led led(ledPinNum);

void setup() {

  // start serial communication
  Serial.begin(9600);
  delay(1000);
  
  redirectSerialToStdio();

}

void loop() {

    // reading command from serial
    char buffer[10] = {0};
      
    // promt to read data
    printf("Waiting for command (led on / led off): \n");
    readLine(buffer, sizeof(buffer));

    // process command introduced by user
    processCommand(led, buffer);
  
}
