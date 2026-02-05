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

// reads a line from stdin into buffer, up to maxLen-1 characters
void readLine(char* buffer, size_t maxLen) {
  size_t i = 0;
  int c = fgetc(stdin);

  // Skip any leftover line endings (e.g., CRLF) from the previous read.
  while (c == '\n' || c == '\r') {
    c = fgetc(stdin);
  }

  if (c == EOF) {
    buffer[0] = '\0';
    return;
  }

  while (i < maxLen - 1) {
    if (c == '\n' || c == '\r' || c == EOF) {
      break;
    }
    buffer[i++] = (char)c;
    c = fgetc(stdin);
  }

  buffer[i] = '\0';

  // Discard the rest of the line if the buffer filled up.
  if (i == maxLen - 1 && c != '\n' && c != '\r' && c != EOF) {
    do {
      c = fgetc(stdin);
    } while (c != '\n' && c != '\r' && c != EOF);
  }

  // If we stopped on CR, consume an optional LF to avoid empty reads.
  // if (c == '\r') {
  //   int next = fgetc(stdin);
  //   if (next != '\n' && next != EOF) {
  //     ungetc(next, stdin);
  //   }
  // }
}

void redirectSerialToStdio() {
  static FILE uartio;

  fdev_setup_stream(&uartio, serial_putchar, serial_getchar,
                    _FDEV_SETUP_READ | _FDEV_SETUP_WRITE);

  stdin = &uartio;
  stdout = &uartio;
  stderr = &uartio;
}
