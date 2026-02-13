#include "PinCodeSystem.h"
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// PASSWORD is defined in main.cpp (stored in PROGMEM)

// Internal maximum buffer for readPassword()
static constexpr uint8_t INTERNAL_BUFFER_SIZE = 32;


void handleThisSupremeSecuredSystem(Led& red, Led& green, uint8_t maxPasswordLength) {

    byte* passwordAttempt = readPassword(maxPasswordLength);

    // if password is correct 
    if (strcmp_P((char*)passwordAttempt, PASSWORD) == 0) {

        // turn on green and print happy message
        green.on();
        red.off();
        clearline();
        printf_P(PSTR("Yappy <3"));
        delay(2000);
        clearline();
    } else {

        // turn on red and print sad message
        red.on();
        green.off();
        clearline();
        printf_P(PSTR("Nope >:("));
        delay(2000);
        clearline();
    }
}

// password should be numeric and not exceed maxPasswordLength
bool isConfiguredPasswordValid(uint8_t maxPasswordLength) {
    uint8_t length = strlen_P(PASSWORD);
    if (length == 0 || length > maxPasswordLength) {
        return false;
    }
    for (uint8_t i = 0; i < length; i++) {
        char c = (char)pgm_read_byte(&PASSWORD[i]);
        if (!isdigit((unsigned char)c)) {
            return false;
        }
    }
    return true;
}

// prints 16 spsaces from the beginning of the line and returns the cursor
static void clearline() {
    putchar('\r');
    // default to a 16-column LCD (matches project's LCD_COLS)
    for (uint8_t i = 0; i < 16; ++i) {
        putchar(' ');
    }
    putchar('\r');
}



// reads password until '#' or is the length succeds maxPassword length
byte* readPassword(uint8_t maxPasswordLength) {
    static byte input[INTERNAL_BUFFER_SIZE + 1];

    if (maxPasswordLength > INTERNAL_BUFFER_SIZE) {
        maxPasswordLength = INTERNAL_BUFFER_SIZE;
    }

    uint8_t i;
    for (i = 0; i < maxPasswordLength; i++) {
        char c;
        scanf("%c", &c);
        if (c == '#') {
            input[i] = '\0';
            break;
        }
        input[i] = c;
        putchar('*');
    }
    input[i] = '\0';
    return input;
}
