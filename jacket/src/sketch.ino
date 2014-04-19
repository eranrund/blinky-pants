#include "FastSPI_LED2.h"

#define LED_COUNT 3 
#define DATA_PIN 9

CRGB leds[LED_COUNT];

void setup()
{
    Serial.begin(9600);
    Serial.println("OK");

    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, LED_COUNT);
    FastLED.setBrightness(128);
    pinMode(DATA_PIN, OUTPUT);

    for (int i = 0; i < LED_COUNT; ++i) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
}

int idx = 0;
void loop() {
    while (Serial.available()) {
        char ch = Serial.read();
        switch (ch) {
            case '=':
               idx ++;
              break;

             case '-': 
                idx--;
                break;

            case '9':
                idx -= 10;
                break;

            case '0':
                idx += 10;
                break;
        }

        if (0 > idx) {
            idx = 0;
        }

        if (idx >= LED_COUNT) {
            idx = LED_COUNT - 1;
        }

        Serial.println(idx);

        memset(leds, 0, sizeof(leds));
        /*for (int i = 0; i < idx; ++i) {
            leds[i] = CRGB::Green;
        }*/
        leds[idx] = CRGB::Green;
        FastLED.show();
    }
}
