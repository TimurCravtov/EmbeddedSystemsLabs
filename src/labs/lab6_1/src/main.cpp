#include <Arduino.h>
#include <button/button.h>
#include <led/led.h>
#include <serialio/serialio.h>

#include "controller/button_led_controller.h"

constexpr uint8_t LED_PIN = 2;
constexpr uint8_t BUTTON_PIN = 5;
constexpr uint16_t DEBOUNCE_MS = 40;
constexpr uint16_t MIN_TOGGLE_INTERVAL_MS = 250;

Led led(LED_PIN);
Button button(BUTTON_PIN);

static ButtonLedController controller(&led, DEBOUNCE_MS, MIN_TOGGLE_INTERVAL_MS);

void setup() {
    led.init();
    button.init();

    Serial.begin(9600);
    redirectSerialToStdio(true, true, true);

    const bool initialRaw = button.isPressed();
    controller.begin(initialRaw, millis());
}

void loop() {
    const uint32_t now = millis();
    const bool rawPressed = button.isPressed();

    controller.update(rawPressed, now);

    delay(5);
}
