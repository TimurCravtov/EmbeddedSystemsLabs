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
  

  // the function does what you think it does
  redirectSerialToStdio();
}

bool equals(const char* a, const char* b) {
  return strcasecmp(a, b) == 0;
}

void loop() {

  // reading command from serial
  char buffer[10] = {0};
  fflush(stdout);
  
  printf("Waiting for command (led on / led off): \n");

  readLine(buffer, sizeof(buffer));

  if (equals(buffer, "led on")) {

    // printf("Intered command to turn LED ON\n");
    if (led.isOn()) {
      printf("LED is already ON\n");
    } else {
      printf("Turning LED ON\n");
      led.on();
    }
  }

  else if (equals(buffer, "led off")) {

    // printf("Intered command to turn LED OFF\n");
    if (!led.isOn()) {
      printf("LED is already OFF\n");
    } else {
      printf("Turning LED OFF\n");
      led.off(); 
    }
  } else {
    printf("Unknown command. Use 'led on' or 'led off'.\n");
  }
}
