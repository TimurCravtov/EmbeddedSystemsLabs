#include <LedController.h>
#include <string.h>

void handleLed(Led& led, bool);

void processCommand(Led& led, char* command) {
    if (strcasecmp_P(command, PSTR("led on")) == 0) {
        handleLed(led, true);
    } else if (strcasecmp_P(command, PSTR("led off")) == 0) {
        handleLed(led, false);
    } else {
        printf_P(PSTR("Unknown command. Use 'led on' or 'led off'.\n"));
    }
}

void handleLed(Led& led, bool turnOn) {
    if (turnOn) {
        if (!led.isOn()) {
            led.on();
            printf_P(PSTR("LED turned ON\n"));
        } else {
            printf_P(PSTR("LED is already ON\n"));
        }
    } else {
        if (led.isOn()) {
            led.off();
            printf_P(PSTR("LED turned OFF\n"));
        } else {
            printf_P(PSTR("LED is already OFF\n"));
        }
    }
}