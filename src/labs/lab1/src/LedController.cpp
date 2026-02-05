#include <LedController.h>
#include <string.h>

void handleLed(Led& led, bool);

void processCommand(Led& led, char* command) {
    if (strcasecmp(command, "led on") == 0) {
        handleLed(led, true);
    } else if (strcasecmp(command, "led off") == 0) {
        handleLed(led, false);
    } else {
        printf("Unknown command. Use 'led on' or 'led off'.\n");
    }
}

void handleLed(Led& led, bool turnOn) {
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