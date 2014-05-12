#include <FastLED.h>

#define N_LEDS 10
#define MAX_V 100
CRGB leds[N_LEDS];

void setup() {
    Serial.begin(9600);
    FastLED.addLeds<WS2812B, 3, GRB>(leds, N_LEDS);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, OUTPUT);

    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
}

void leds_on() {
    for (int v = 0; v < MAX_V; ++v) {
        for (int i = 0; i < N_LEDS; ++i) {
            leds[i] = CHSV(0, 255, v);
        }
        FastLED.show();
        delay(10);
    }
}

void leds_off() {
    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CRGB::Black;
    }
    FastLED.show();
}

bool state = false;
unsigned long last_switched_at = 0;


unsigned long x = 0;
unsigned long x2 = 0;
void loop() {
    int sensor_read = digitalRead(2);

    /* if ((x + 1000) < millis()) { */
    /*     Serial.println(x2++); */
    /*     x = millis(); */
    /* } */

    if (sensor_read) {
        last_switched_at = millis();

        if (!state) {    
            state = true;
            leds_on();
            Serial.println("ON");
        } else {
            Serial.println("STILL ON");
        }
    }

    if (state && !sensor_read && ((last_switched_at + 30000) < millis())) {
        state = false;
        leds_off();
        Serial.println("OFF");
        delay(2000);
    }
}
