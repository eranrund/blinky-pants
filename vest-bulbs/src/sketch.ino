#include <FastLED.h>

#define LED_PIN     2
#define NUM_LEDS    30
#define N_LEDS NUM_LEDS
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
CRGB * leds2 = (leds + (N_LEDS/2));

CRGB ledsX[N_LEDS];
#define copy_led_array() memcpy(ledsX, leds, sizeof(leds))
#define uncopy_led_array() memcpy(leds, ledsX, sizeof(leds))

#define UPDATES_PER_SECOND 100

// This example shows several ways to set up and use 'palettes' of colors
// with FastLED.
//
// These compact palettes provide an easy way to re-colorize your
// animation on the fly, quickly, easily, and with low overhead.
//
// USING palettes is MUCH simpler in practice than in theory, so first just
// run this sketch, and watch the pretty lights as you then read through
// the code.  Although this sketch has eight (or more) different color schemes,
// the entire sketch compiles down to about 6.5K on AVR.
//
// FastLED provides a few pre-configured color palettes, and makes it
// extremely easy to make up your own color schemes with palettes.
//
// Some notes on the more abstract 'theory and practice' of
// FastLED compact palettes are at the bottom of this file.



CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


void setup() {
    delay( 1500 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}

void ChangePalettePeriodically();
void FillLEDsFromPaletteColors( uint8_t colorIndex);
void SetupPurpleAndGreenPalette();
void SetupTotallyRandomPalette();
void SetupBlackAndWhiteStripedPalette();


class StepGenerator {
public:
    const unsigned char * pattern;
    unsigned char num_steps;
    unsigned char step;

    StepGenerator(const unsigned char * pattern, unsigned char num_steps) {
        this->pattern = pattern;
        this->num_steps = num_steps;
        this->step = 0;
    }

    unsigned char next() {
        unsigned char ret = pattern[step];
        step = (step + 1) % num_steps;
        return ret;
    }
};

class BasePattern {
public:
    unsigned long step, max_steps;

    BasePattern(unsigned long max_steps) {
        this->step = 0;
        this->max_steps = max_steps;
    }

    void loop() {
        step++;
        if (step == max_steps) {
            step = 0;
        }
    }

    bool next() {
        return step == (max_steps - 1);
    }

    // This function fades val by decreasing it by an amount proportional
    // to its current value.  The fadeTime argument determines the
    // how quickly the value fades.  The new value of val will be:
    //   val = val - val*2^(-fadeTime)
    // So a smaller fadeTime value leads to a quicker fade.
    // If val is greater than zero, val will always be decreased by
    // at least 1.
    // val is a pointer to the byte to be faded.
    void fade(unsigned char *val, unsigned char fadeTime) {
        if (*val != 0) {
            unsigned char subAmt = *val >> fadeTime;  // val * 2^-fadeTime
            if (subAmt < 1) {
                subAmt = 1;  // make sure we always decrease by at least 1
            }
            *val -= subAmt;  // decrease value of byte pointed to by val
        }
    }
};

class FaderPattern1 : public BasePattern {
public:
    int v;
    bool dir;

    int balance;
    bool balance_dir;

    FaderPattern1() : BasePattern(0xffffffff) {
        v = 0;
        dir = true;
        balance = 0;
        balance_dir = true;
    }

   void loop1() {
        for (int i = 0; i < N_LEDS; ++i) {
            leds[i] = CHSV(step >> 4, 255,
                    200 + (55 * sin(( (step % 200) * 2 * PI / 200)))
            );
        }

        inc();
        delay(1);
    }

    void loop2() {
        int state = (step >> 9) % 9;

        loop2(
            (state >> 0) & 1,
            (state >> 1) & 1,
            (state >> 2) & 1
        );
    }

     void loop2(bool compl_colors, bool cos_sin1, bool interleave) {
        for (int i = 0; i < N_LEDS / 2; ++i) {
            //leds[i] = CHSV(step >> 5, 255, 155 + (balance / 2));
            //leds[i+ (N_LEDS/2)] = CHSV(step >> 5, 255, 255 - (balance / 2));
            leds[interleave ? i * 2 : i] = CHSV(step >> 4, 255,
                    200 + (55 * sin(( (step % 150) * 2 * PI / 150)))
            );
            leds[interleave ? (i * 2) + 1 : i + (N_LEDS/2)] = CHSV(
                    (compl_colors ? 128 : 0) + (step >> 4), 255,
                    200 + (-55 * (cos_sin1 ? sin : cos)( (step % 150) * 2 * PI / 150))
            );
        }

        inc();
        delay(4);
    }

    void loop3() {        
        static const unsigned char pattern[] = {0, 64, 128, 255, 128, 64, 0};
        static StepGenerator gen1(pattern, sizeof(pattern)/sizeof(pattern[0]));        
        static unsigned char next_h = 0;

        unsigned char next_v = gen1.next();
        if (next_v == 255) {
            next_v -= random(35);
        }

        if (gen1.step == 0) {
            next_h = random(255);
        }


        memmove(leds + 1, leds, sizeof(leds[0]) * ((N_LEDS / 2) - 1));
        memmove(leds2 + 1, leds2, sizeof(leds[0]) * ((N_LEDS / 2) - 1));
        
        leds[0] = CHSV(       
            next_h,
            255,
            next_v
        );

        leds2[0] = leds[0];

        inc();
        delay(20 * sin( (step % 150) * 2*PI / 150 ) + 60);
    }

    void inc() {
        BasePattern::loop();
        FastLED.show();

        v += dir ? 1 : -1;
        if ((v == 100) || (v == 0)) {
            dir = !dir;
        }

        balance += balance_dir ? 1 : -1;
        if ((balance == 0) || (balance == 200)) {
            balance_dir = !balance_dir;
        }

    }
};
FaderPattern1 FaderPattern1;

class CollisionPattern : public BasePattern {
public:
    unsigned char state;  // pattern state
    unsigned int count;  // counter used by pattern

    CollisionPattern() : BasePattern(0) {
        state = 0;
        count = 0;
    }

    void loop() {
        BasePattern::loop();

        if (!collision())
        {
            max_steps = step + 2;
        }
        FastLED.show();
        delay(10);
    }

    unsigned char collision()
    {
        const unsigned char maxBrightness = 210;  // max brightness for the colors
        const unsigned char numCollisions = 5;  // # of collisions before pattern ends
          if (step == 0)
          {
            state = 0;
          }

          if (state % 3 == 0)
          {
            // initialization state
            switch (state/3)
            {
              case 0:  // first collision: red streams
                leds[0] = CRGB(maxBrightness, 0, 0);
                break;
              case 1:  // second collision: green streams
                leds[0] = CRGB(0, maxBrightness, 0);
                break;
              case 2:  // third collision: blue streams
                leds[0] = CRGB(0, 0, maxBrightness);
                break;
              case 3:  // fourth collision: warm white streams
                leds[0] = CRGB(maxBrightness, maxBrightness*4/5, maxBrightness>>3);
                break;
              default:  // fifth collision and beyond: random-color streams
                leds[0] = CRGB(random(maxBrightness), random(maxBrightness), random(maxBrightness));
            }

            // stream is led by two full-white LEDs
            leds[1] = leds[2] = CRGB(255, 255, 255);
            // make other side of the strip a mirror image of this side
            leds[N_LEDS - 1] = leds[0];
            leds[N_LEDS - 2] = leds[1];
            leds[N_LEDS - 3] = leds[2];

            state++;  // advance to next state
            count = 8;  // pick the first value of count that results in a startIdx of 1 (see below)
            return 0;
          }

          if (state % 3 == 1)
          {
            // stream-generation state; streams accelerate towards each other
            unsigned int startIdx = count*(count + 1) >> 6;
            unsigned int stopIdx = startIdx + (count >> 5);
            count++;
            if (startIdx < (N_LEDS + 1)/2)
            {
              // if streams have not crossed the half-way point, keep them growing
              for (int i = 0; i < startIdx-1; i++)
              {
                // start fading previously generated parts of the stream
                fade(&leds[i].red, 5);
                fade(&leds[i].green, 5);
                fade(&leds[i].blue, 5);
                fade(&leds[N_LEDS - i - 1].red, 5);
                fade(&leds[N_LEDS - i - 1].green, 5);
                fade(&leds[N_LEDS - i - 1].blue, 5);
              }
              for (int i = startIdx; i <= stopIdx; i++)
              {
                // generate new parts of the stream
                if (i >= (N_LEDS + 1) / 2)
                {
                  // anything past the halfway point is white
                  leds[i] = CRGB(255, 255, 255);
                }
                else
                {
                  leds[i] = leds[i-1];
                }
                // make other side of the strip a mirror image of this side
                leds[N_LEDS - i - 1] = leds[i];
              }
              // stream is led by two full-white LEDs
              leds[stopIdx + 1] = leds[stopIdx + 2] = CRGB(255, 255, 255);
              // make other side of the strip a mirror image of this side
              leds[N_LEDS - stopIdx - 2] = leds[stopIdx + 1];
              leds[N_LEDS - stopIdx - 3] = leds[stopIdx + 2];
            }
            else
            {
              // streams have crossed the half-way point of the strip;
              // flash the entire strip full-brightness white (ignores maxBrightness limits)
              for (int i = 0; i < N_LEDS; i++)
              {
                leds[i] = CRGB(255, 255, 255);
              }
              state++;  // advance to next state
            }
            return 0;
          }

          if (state % 3 == 2)
          {
            // fade state
            if (leds[0].red == 0 && leds[0].green == 0 && leds[0].blue == 0)
            {
              // if first LED is fully off, advance to next state
              state++;

              // after numCollisions collisions, this pattern is done
              return state == 3*numCollisions;
            }

            // fade the LEDs at different rates based on the state
            for (int i = 0; i < N_LEDS; i++)
            {
              switch (state/3)
              {
                case 0:  // fade through green
                  fade(&leds[i].red, 3);
                  fade(&leds[i].green, 4);
                  fade(&leds[i].blue, 2);
                  break;
                case 1:  // fade through red
                  fade(&leds[i].red, 4);
                  fade(&leds[i].green, 3);
                  fade(&leds[i].blue, 2);
                  break;
                case 2:  // fade through yellow
                  fade(&leds[i].red, 4);
                  fade(&leds[i].green, 4);
                  fade(&leds[i].blue, 3);
                  break;
                case 3:  // fade through blue
                  fade(&leds[i].red, 3);
                  fade(&leds[i].green, 2);
                  fade(&leds[i].blue, 4);
                  break;
                default:  // stay white through entire fade
                  fade(&leds[i].red, 4);
                  fade(&leds[i].green, 4);
                  fade(&leds[i].blue, 4);
              }
            }
  }

  return 0;
}
};
CollisionPattern CollisionPattern;

void orig_loop()
{
    //ChangePalettePeriodically();
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void loop() {
    static unsigned long last_switch_at = 0;
    static unsigned char pattern = 1;
    unsigned long pattern_time = 0;
     static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    switch (pattern) {
        case 0:
            FaderPattern1.loop1();
            pattern_time = 5000;
            break;

        case 1:
            FaderPattern1.loop2();
            pattern_time = 43000;
            break;

        case 2:
            FaderPattern1.loop3();
            pattern_time = 10000;
            break;

        case 3:
          CollisionPattern.loop();
          pattern_time = 1000;
          break;

         case 4: currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; pattern_time = 10000; orig_loop(); break;
          case 5: currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  pattern_time = 10000; orig_loop(); break;
          case 6: currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; pattern_time = 10000; orig_loop(); break;
          case 7: SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; pattern_time = 10000; orig_loop(); break;
        case 8: currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; pattern_time = 10000; orig_loop(); break;

    }


    if (millis() > (last_switch_at + pattern_time)) {
        pattern = (pattern + 1) % 9;
        last_switch_at = millis();
    }
}




void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 40;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 35)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }

    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}


// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}




// Additionl notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact 
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved 
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.
