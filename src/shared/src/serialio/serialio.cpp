#include <Arduino.h>
#include <serialio/serialio.h>
#include <stdio.h>

#include <stdlib.h>


// add a char to serial output
int serial_putchar(char c, FILE* stream) {
  Serial.write(c);
  return 0;
}

// get a char from serial input
int serial_getchar(FILE* stream) {
  while (!Serial.available());
  return Serial.read();
}

// Helper functions for redirecting Serial to stdio
void redirectSerialToStdio() {
  static FILE uartinout;

  fdev_setup_stream(&uartinout, serial_putchar, serial_getchar, _FDEV_SETUP_RW);
  
  stdout = stdin = stderr= &uartinout;
}

