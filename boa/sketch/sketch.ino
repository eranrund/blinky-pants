#include <avr/sleep.h>
#include "FastLED.h"

#define LED1_PIN   15 // PB7


#define NUM_LEDS           83
#define UPDATES_PER_SECOND 100

CRGB leds[NUM_LEDS];
CRGB leds_off[NUM_LEDS];
uint8_t brightness = 64;

void setup()
{
    Serial.begin(9600);

    delay(2000);

    FastLED.addLeds<WS2812B, LED1_PIN, GRB>(leds, NUM_LEDS);
	FastLED.setBrightness(brightness);

    pinMode(LED1_PIN, OUTPUT);
}

#include "patterns_tinybee.h"
int last_brightness = 0;

void loop()

{

      int brightness = analogRead(0) >> 2;
    if (brightness != last_brightness) {
        last_brightness = brightness;
        FastLED.setBrightness(255 - last_brightness);
    }

    
    /*static int i = 0;
    Serial.print(i++);
    Serial.print(" ");
    Serial.print(brightness); Serial.print(" " );
    //Serial.print(digitalRead(BTN1_PIN)); Serial.print(" ");
    //Serial.print(digitalRead(BTN2_PIN)); Serial.print(" ");
    //Serial.print(digitalRead(BTN3_PIN)); Serial.print(" ");
    //Serial.print(digitalRead(BTN4_PIN));
    Serial.println();
    */


    tinybee_loop();

#if 0
	/*****/
	static uint8_t startIndex = 0;
    startIndex = startIndex + 1;
           FillLEDsFromPaletteColors( startIndex);
    /*****/

	//leds[0] = true ? CRGB::Green : CRGB::Blue; //CHSV(i, 255, 255);
	//leds[1] = CRGB::Red;

	FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
#endif
}


CRGBPalette16 currentPalette = RainbowColors_p;
TBlendType    currentBlending = LINEARBLEND;
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}
