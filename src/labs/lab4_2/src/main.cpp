{
  "version": 1,
  "author": "Timur",
  "editor": "wokwi",
  "parts": [
    { "type": "wokwi-arduino-mega", "id": "mega", "top": 300, "left": -200, "attrs": {} },
    { "type": "wokwi-led", "id": "led1", "top": 50, "left": 100, "attrs": { "color": "red" } },
    {
      "type": "wokwi-resistor",
      "id": "res1",
      "top": 100,
      "left": 100,
      "attrs": { "value": "220" }
    },
    { "type": "wokwi-servo", "id": "servo", "top": 50, "left": 350, "attrs": {} },
    {
      "type": "wokwi-lcd1602-i22",
      "id": "lcd",
      "top": -15.77,
      "left": -195.2,
      "attrs": { "i2cAddress": "0x27" }
    },
    { "type": "wokwi-membrane-keypad", "id": "keypad", "top": 400, "left": 500, "attrs": {} },
    {
      "type": "wokwi-lcd1602",
      "id": "lcd1",
      "top": 83.2,
      "left": -244,
      "attrs": { "pins": "i2c" }
    }
  ],
  "connections": [
    [ "mega:10", "res1:1", "green", [ "h0" ] ],
    [ "res1:2", "led1:A", "green", [ "h0" ] ],
    [ "mega:GND.1", "led1:C", "black", [ "v-20", "h-50" ] ],
    [ "mega:GND.2", "servo:GND", "black", [ "v-100", "h200" ] ],
    [ "mega:5V", "servo:VCC", "red", [ "v-80", "h200" ] ],
    [ "mega:11", "servo:PWM", "orange", [ "v-60", "h200" ] ],
    [ "mega:GND.3", "lcd:GND", "black", [ "v20", "h400" ] ],
    [ "mega:5V.2", "lcd:VCC", "red", [ "v40", "h400" ] ],
    [ "mega:20", "lcd:SDA", "blue", [ "v-30", "h400" ] ],
    [ "mega:21", "lcd:SCL", "purple", [ "v-50", "h400" ] ],
    [ "keypad:R1", "mega:9", "orange", [ "h-200" ] ],
    [ "keypad:R2", "mega:8", "orange", [ "h-200" ] ],
    [ "keypad:R3", "mega:7", "orange", [ "h-200" ] ],
    [ "keypad:R4", "mega:6", "orange", [ "h-200" ] ],
    [ "keypad:C1", "mega:5", "blue", [ "h-200" ] ],
    [ "keypad:C2", "mega:4", "blue", [ "h-200" ] ],
    [ "keypad:C3", "mega:3", "blue", [ "h-200" ] ],
    [ "keypad:C4", "mega:2", "blue", [ "h-200" ] ],
    [ "mega:GND.4", "lcd1:GND", "black", [ "v-150" ] ],
    [ "mega:5V.3", "lcd1:VCC", "red", [ "v-170" ] ],
    [ "mega:20", "lcd1:SDA", "blue", [ "v-190" ] ],
    [ "mega:21", "lcd1:SCL", "purple", [ "v-210" ] ]
  ],
  "dependencies": {}
}