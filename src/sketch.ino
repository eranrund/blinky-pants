#define LED_PIN 13

#define FORCE_SOFTWARE_SPI
#define FORCE_SOFTWARE_PINS
#include "FastSPI_LED2.h"


#define SEG1_BEGIN 0
#define SEG1_END 26
#define SEG2_BEGIN 27
#define SEG2_END 50
#define SEG3_BEGIN 51
#define SEG3_END 75
#define SEG4_BEGIN 76
#define SEG4_END 106

#define NUM_LEDS 106
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

void setup()
{
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(128);
    pinMode(LED_PIN, OUTPUT);

    for (int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
}



unsigned long tick = 0;
uint16_t i, j, x, y;
uint32_t c, d;

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128; // red down
      g = WheelPos % 128;       // green up
      b = 0;                    // blue off
      break;
    case 1:
      g = 127 - WheelPos % 128; // green down
      b = WheelPos % 128;       // blue up
      r = 0;                    // red off
      break;
    case 2:
      b = 127 - WheelPos % 128; // blue down
      r = WheelPos % 128;       // red up
      g = 0;                    // green off
      break;
  }
  return(CRGB(r,g,b));
}

void animate() {
    j = tick % 384;
    for (i=0; i <NUM_LEDS; i++) {
        leds[i] =  Wheel(((i * 384 / NUM_LEDS * (2)) + j) % 384);
    }
    FastLED.show();
}

void loop() {
    tick++;
    animate();
    //handleButtons();
}


// This function runs over and over, and is where you do the magic to light
// your leds.
void xxloop() {
    delay(700);
    for (int i = SEG4_BEGIN; i < SEG4_END; ++i) {
        leds[i] = CRGB::Green;
    }
    FastLED.show();
    delay(700);
    for (int i = SEG4_BEGIN; i < SEG4_END; ++i) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();


   // Move a single white led
/*   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::Red;
      leds[whiteLed+1] = CRGB::Green;
      leds[whiteLed+2] = CRGB::Blue;
      //
      // Show the leds (only one of which is set to white, from above)
      FastLED.show();

      // Wait a little bit
      delay(10);

      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }*/
}

/*void loop()
{
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(900);
}*/
