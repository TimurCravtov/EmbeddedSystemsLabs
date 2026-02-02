#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <led/led.h> 
#include <serialio/serialio.h>
#include <string.h>


// setup LED on pin 13
const uint8_t ledPinNum = 13;
Led led(ledPinNum);

void setup() {

  // start serial communication
  Serial.begin(9600);
  delay(100);
  
  redirectSerialToStdio();
}

bool equalsIgnoreCase(const char* a, const char* b) {
  return strcasecmp(a, b) == 0;
}

void loop() {

    // reading command from serial
    char buffer[10] = {0};
    fflush(stdout);
  
    // promt to read data
    printf("Waiting for command (led on / led off): \n");
    readLine(buffer, sizeof(buffer));

    // process command
    if (equalsIgnoreCase(buffer, "led on")) handleLed(true);
    else if (equalsIgnoreCase(buffer, "led off")) handleLed(false);
    else printf("Unknown command. Use 'led on' or 'led off'.\n");
  
}

void handleLed(bool turnOn) {
  if (turnOn) {
    if (!led.isOn()) {
      led.on();
      printf("LED turned ON\n");
    } else {
      printf("LED is already ON\n");
    }
  } else {
    if (led.isOn()) {
      led.off();
      printf("LED turned OFF\n");
    } else {
      printf("LED is already OFF\n");
    }
  }
}