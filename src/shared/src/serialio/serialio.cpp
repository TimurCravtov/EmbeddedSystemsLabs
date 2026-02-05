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

// reads a line from stdin into buffer, up to maxLen-1 characters, and discards the rest of the line if too long
void readLine(char* buffer, size_t maxLen, bool discardRestOfLine = true) {
  size_t i = 0;
  while (i < maxLen - 1) {
    int c = fgetc(stdin);
    if (c == '\n' || c == '\r' || c == EOF) {
      break;
    }
    buffer[i++] = (char)c;
  }

  buffer[i] = '\0';

  // discard the rest of the line if requested
  if (i == maxLen - 1 && discardRestOfLine) {
    int c;
    do {
      c = fgetc(stdin);
    } while (c != '\n' && c != '\r' && c != EOF);
  }
}

// Helper functions for redirecting Serial to stdio
void redirectSerialToStdio() {
  static FILE uartout;
  static FILE uartin;

  // setup the UART streams for input and output
  fdev_setup_stream(&uartout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  fdev_setup_stream(&uartin, NULL, serial_getchar, _FDEV_SETUP_READ);
  
  // redirect standard input and output to the UART
  stdout = &uartout;
  stdin = &uartin;
  stderr = &uartout; // never used this, but why not. output errors to serial as well
}

