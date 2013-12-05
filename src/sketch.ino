#define LED_PIN 13

#define FORCE_SOFTWARE_SPI
#define FORCE_SOFTWARE_PINS
#include "FastSPI_LED2.h"

#define NUM_LEDS 150
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

void setup()
{
    delay(2000);

    FastLED.addLeds<UCS1903, DATA_PIN, GRB>(leds, NUM_LEDS);
    pinMode(LED_PIN, OUTPUT);
}

// This function runs over and over, and is where you do the magic to light
// your leds.
void loop() {
   // Move a single white led
   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::Red;
      leds[whiteLed+1] = CRGB::Green;
      leds[whiteLed+2] = CRGB::Blue;
      //
      // Show the leds (only one of which is set to white, from above)
      FastLED.show();

      // Wait a little bit
      delay(1000);

      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }
}

/*void loop()
{
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(900);
}*/
