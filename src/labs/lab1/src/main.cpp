#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <led/led.h> 
#include <serialio/serialio.h>

const uint8_t ledPinNum = 13;

Led led(ledPinNum);

void setup() {

  Serial.begin(9600);
  delay(100);
  
  redirectSerialToStdio();
}

bool equals(const char* a, const char* b) {
  return strcmp(a, b) == 0;
}

void loop() {

  char buffer[100] = {0};
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
