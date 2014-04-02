#include <MemoryFree.h>
#include "FastSPI_LED2.h"
#include <EEPROM.h>
#include <Encoder.h>
#include "BlinkyCommon.h"
#include "BlinkyFaderPattern.h"


////////////////////////////////////////////////////////////////////////////////
// Config
////////////////////////////////////////////////////////////////////////////////

const LedRange rings[] = {
    // left
    {0, 17},
    {20, 35},
    {38, 51},
    {55, 67},
    {70, 82},
    {85, 95},

    // right
    {96, 113},
    {116, 131},
    {134, 148},
    {152, 164},
    {167, 178},
    {181, 191},
};
#define n_rings arr_len(rings)

const LedRange lines[] = {
    // left
    {18, 18}, // 1
    {19, 20}, // 2
    {36, 38}, // 3
    {52, 55}, // 3
    {68, 69}, // 2
    {83, 84}, // 3

    // right
    {96, 96},   // 1
    {114, 116}, // 3
    {131, 134}, // 4
    {149, 151}, // 3
    {165, 167}, // 3
    {179, 181}, // 3
};
#define n_lines arr_len(lines)

#define DATA_PIN 9
#define ENC1_PIN1 2
#define ENC1_PIN2 3
#define ENC1_SW_PIN 4
#define BRIGHTNESS_PIN A0
#define SPEED_PIN A1

#define MAX_LEDS 192


CRGB leds[MAX_LEDS];
CRGB ledsX[MAX_LEDS];
CRGB * leds2;
#define copy_led_array() memcpy(ledsX, leds, sizeof(leds))
#define uncopy_led_array() memcpy(leds, ledsX, sizeof(leds))


int PANTS_VERSION = 2;
int N_LEDS;


//
unsigned char g_brightness = 16;
unsigned char g_speed = 10;
unsigned char g_pattern = 0;
unsigned long g_pattern_duration = 0;
unsigned long g_pattern_last_switch_at = 0;
unsigned char g_num_patterns = 0;
 


//
unsigned int seed = 0;



////////////////////////////////////////////////////////////////////////////////
// Pattern Instances
////////////////////////////////////////////////////////////////////////////////
BlinkyFaderPattern1 FaderPattern1;


inline void FaderPattern1_loop1() { FaderPattern1.loop1(); }
inline void FaderPattern1_loop2_0() { FaderPattern1.loop2(false, false, false); }
inline void FaderPattern1_loop2_1() { FaderPattern1.loop2(true, false, false); }
inline void FaderPattern1_loop2_2() { FaderPattern1.loop2(false, true, false); }
inline void FaderPattern1_loop2_3() { FaderPattern1.loop2(true, true, false); }
inline void FaderPattern1_loop2_4() { FaderPattern1.loop2(false, false, true); }
inline void FaderPattern1_loop2_5() { FaderPattern1.loop2(true, false, true); }
inline void FaderPattern1_loop2_6() { FaderPattern1.loop2(false, true, true); }
inline void FaderPattern1_loop2_7() { FaderPattern1.loop2(true, true, true); }
inline void FaderPattern1_loop3() { FaderPattern1.loop3(); }

const PatternInstance PantsV2_PatternInstances[] = {
    {FaderPattern1_loop1, 5000},
    {FaderPattern1_loop2_0, 5375},
    {FaderPattern1_loop2_1, 5375},
    {FaderPattern1_loop2_2, 5375},
    {FaderPattern1_loop2_3, 5375},
    {FaderPattern1_loop2_4, 5375},
    {FaderPattern1_loop2_5, 5375},
    {FaderPattern1_loop2_6, 5375},
    {FaderPattern1_loop2_7, 5375},
    {FaderPattern1_loop3, 10000},
};

const PatternInstance * g_patterns;

void switch_patterns(const PatternInstance * patterns, int n_patterns) {
    g_patterns = patterns;
    g_num_patterns = n_patterns;
    goto_pattern(0);
}

// "Manager"
void goto_pattern(unsigned char p) {
    g_pattern = p;

    for (int i = 0; i < 8; i++)
    {
        seed += analogRead(i);
    }
    randomSeed(seed);

    memset(leds, 0, sizeof(leds));
    memset(ledsX, 0, sizeof(ledsX));
    FastLED.show();

    Serial.print("P:");
    Serial.println(g_pattern);
    
    g_pattern_last_switch_at = millis();
    g_pattern_duration = g_patterns[g_pattern].duration;
}

void advance_pattern(bool dir) {
    if (dir) {
        g_pattern = ((unsigned char)(g_pattern + 1)) % g_num_patterns;
    } else {
        if (g_pattern == 0) {
            g_pattern = g_num_patterns- 1;
        } else {
            --g_pattern;
        }
    }
    
    goto_pattern(g_pattern);
}

////////////////////////////////////////////////////////////////////////////////
// enc1
////////////////////////////////////////////////////////////////////////////////

Encoder enc1(ENC1_PIN1,ENC1_PIN2);
bool enc1_btn = false;
unsigned long enc1_btn_pressed_at = 0;


void enc1_moved_without_btn(bool dir)
{
    advance_pattern(dir);
}

void enc1_moved_with_btn(bool dir)
{
}



////////////////////////////////////////////////////////////////////////////////
// setup
////////////////////////////////////////////////////////////////////////////////

// initialization stuff
void setup()
{
    Serial.begin(9600);
    Serial.println("OK");

    for (int i = 0; i < 8; i++)
    {
        seed += analogRead(i);
    }
    seed += EEPROM.read(0);  // get part of the seed from EEPROM
    randomSeed(seed);
    EEPROM.write(0, random(256));


    // GET VERSION
    // D10 - PULL UP = v2
    //       DOWN TO GND = v1
    pinMode(10, INPUT_PULLUP);
    delay(30);
    PANTS_VERSION = digitalRead(10) == 1 ? 2 : 1;

    pinMode(ENC1_SW_PIN, INPUT_PULLUP);

    if (PANTS_VERSION == 1)
    {
        N_LEDS = 106;
        // TODO NUM_STATES = 13;
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, N_LEDS);
    }    
    else if (PANTS_VERSION == 2)
    {
        N_LEDS = 192;
        // TODO NUM_STATES = 17;
        FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, N_LEDS);        
    }
    leds2 = &(leds[N_LEDS/2]);

    FastLED.setBrightness(128);
    pinMode(DATA_PIN, OUTPUT);

    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();

    switch_patterns(PantsV2_PatternInstances, arr_len(PantsV2_PatternInstances));
}



////////////////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////////////////

inline void loop_brightness()
{
    unsigned char brightness;

    // TODO
    /*
#if PANTS_VERSION == 1
    brightness = (unsigned char) (analogRead(BRIGHTNESS_PIN) >> 2);
//    brightness = 32; // TODO
    if (abs(brightness - g_brightness) > 5) {
        if (brightness < 7) {
            FastLED.setBrightness(0);
        } else if (brightness > 249) {
            FastLED.setBrightness(255);
        } else {            
            FastLED.setBrightness(brightness);
        }
        Serial.print("Brightness: ");
        Serial.println(brightness);
        g_brightness = brightness;
    }
#elif PANTS_VERSION == 2*/
    brightness = (unsigned char) (analogRead(BRIGHTNESS_PIN) >> 3);
    if (brightness != g_brightness) {
        FastLED.setBrightness(brightness);
        g_brightness = brightness;
    }
    /*
#else
    brightness = 32;
    if (brightness != g_brightness) {
        FastLED.setBrightness(brightness);
        g_brightness = brightness;
    }
#endif*/
}

inline void loop_rotenc1()
{
    static long enc_pos = 0;
    static unsigned long last_change = 0;
    static bool waiting_for_long_press = false;
    long new_pos;
    unsigned long now;

    int sw = digitalRead(ENC1_SW_PIN);
    while (sw != digitalRead(ENC1_SW_PIN)) {
        sw = digitalRead(ENC1_SW_PIN);
        delay(1);
        Serial.println("%");
    }
    if ((sw == 1) & enc1_btn) {
        // button released
        enc1_btn = false;
        waiting_for_long_press = false;
        Serial.println("ENC1 BTN -");
        delay(350);
        enc_pos = enc1.read();


        return;
    } else if ((sw == 0) && !enc1_btn) {
        // button pressed
        enc1_btn = true;
        waiting_for_long_press = true;
        enc1_btn_pressed_at = millis();
        Serial.println("ENC1 BTN +");
    }

    new_pos = enc1.read();
    if (enc_pos == new_pos) {
        if (waiting_for_long_press && enc1_btn && ( (millis() - enc1_btn_pressed_at) > 3000)) {
            /*if (mic_mode == MicOff) {
                efx_blink(60, 2);
                mic_mode = MicP1;
            } else {
                efx_blink(90, 2);
                mic_mode = MicOff;
            }
            TODO
            */
        }
        return;
    }
    waiting_for_long_press = false; // rotenc moved so no longpress

    Serial.println(enc_pos);
    now = millis();
    if ((now - last_change) > 300) {
        last_change = now;

        
        if (enc1_btn) {             
            enc1_moved_with_btn(new_pos > enc_pos);
        } else {
            // TODO if (mic_mode == MicOff) {
                enc1_moved_without_btn(new_pos > enc_pos);
            /* TODO } else {
                int dir = new_pos > enc_pos ? 1 : -1;
                mic_mode += dir;
                if (mic_mode == 0) {
                    mic_mode = MicStates - 1;
                } else if (mic_mode == MicStates) {
                    mic_mode = 1;
                }                
            }*/
        }
    }
    enc_pos = new_pos;
}


inline void loop_serial() {
    unsigned char ch;
    while (Serial.available()) {
        ch = Serial.read();
        switch (ch) {
            case 'a':
                // TODO pattern_auto_inc = !pattern_auto_inc;

                /* fallthrough */                
            case '?':
                Serial.print("Version: ");
                Serial.println(PANTS_VERSION);

 /*               Serial.print("Pattern: ");  TODO 
                if (pattern_auto_inc) {
                   Serial.print("autoinc ");
                }
                Serial.println(pattern);*/

                Serial.print("Speed: ");
                Serial.println(g_speed);

                Serial.print("Brightness: ");
                Serial.println(g_brightness);

                Serial.print("Free mem: ");
                Serial.println(freeMemory());
                break;

            case '+':
                advance_pattern(true);
                break;

            case '-':
                advance_pattern(false);
                break;
        }
    }
}


inline void loop_speed() {
    g_speed = (analogRead(SPEED_PIN) >> 3);
}

inline void loop_pattern() {       
    g_patterns[g_pattern].loop();

    if (millis() > (g_pattern_last_switch_at + g_pattern_duration)) {
        advance_pattern(true);
    }

}

void loop() {
    loop_brightness();
    loop_rotenc1();
    loop_serial();
    loop_speed();    
    loop_pattern();
}












void efx_blink(int h, int repeats);
void goto_pattern(unsigned char p);
void advance_pattern(bool dir);
void micbar1loop();
void micbar2loop();
void SymSimpleHSV_pat();
void EMS_pat();
void Flicker_pat();
void RandomMartch_pat();
void Flame_pat();
void Matrix_pat();
void warmWhiteShimmer(unsigned char dimOnly);
void randomColorWalk(unsigned char initializeColors, unsigned char dimOnly);
void traditionalColors();
void colorExplosion(unsigned char noNewBursts);
void gradient();
void brightTwinkle(unsigned char minColor, unsigned char numColors, unsigned char noNewBursts);
unsigned char collision();
void RingsHSV_Loop();
void ShootRings_Loop();
void SpinningRings_Loop(bool sym);

int NUM_STATES;

////


// system timer, incremented by one every time through the main loop
unsigned int loopCount = 0;


// enumerate the possible patterns in the order they will cycle
enum Pattern {
    SymSimpleHSV, // 0
    EMS, // 1
    Flicker, // 2
    RandomMarch, // 3
    Flame, // 4
    Matrix, //5 
    WarmWhiteShimmer , // 6
    RandomColorWalk, // 7
    TraditionalColors, // 8 
    ColorExplosion, // 9
    Gradient, // 10
    BrightTwinkle, // 11
    Collision, // 12

    //NUM_STATES_V1,

    RingsHSV, // 13
    ShootRings, // 14
    SpinningRings, // 15
    SpinningRingsSym, //16

    //NUM_STATES_V2,

    AllOff = 255
};

typedef enum {
    MicOff,
    MicP1,
    MicP2,
    MicStates,
} MicMode;

unsigned char pattern = 0;
bool pattern_auto_inc = true;
//unsigned char pattern = SpinningRings;
//bool pattern_auto_inc = false;
unsigned int maxLoops;  // go to next state when loopCount >= maxLoops
unsigned char mic_mode = MicOff;

// main loop

unsigned char enc_pattern_state = 0;
void old_enc1_moved_with_btn(bool dir)
{    

    if (dir) {
        // move to next, if possible
        if (enc_pattern_state < NUM_STATES) {
            enc_pattern_state++;
        } else {
            copy_led_array();
            efx_blink(0, 2);
            uncopy_led_array();
        }
    } else {
        // move back if possible
        if (enc_pattern_state > 0) {
            enc_pattern_state--;
        } else {
            copy_led_array();
            efx_blink(0, 2);
            uncopy_led_array();
        }
    }

    if (enc_pattern_state == 0) {
        goto_pattern(0);
        pattern_auto_inc = true;
    } else {
        goto_pattern(enc_pattern_state - 1);
        pattern_auto_inc = false;
    }

    Serial.print("Rot enc_pattern_state: ");
    Serial.println(enc_pattern_state);
}

inline void speed_delay(int speed, int delay_time) {
   delay(map(speed, 0, 127, 0, delay_time * 7));
}
void old_loop()
{
    loop_brightness();
    loop_rotenc1();
    loop_serial();
    loop_speed();    

   return; 

    // mic mode hack
    switch (mic_mode) {
        case MicP1:
            micbar1loop();
            return;

        case MicP2:
            micbar2loop();
            return;
    }

    // Clear leds on start of loop
    if (loopCount == 0) {
        memset(leds, 0, sizeof(leds));
    }

    if (pattern == WarmWhiteShimmer || pattern == RandomColorWalk) {
        // for these two patterns, we want to make sure we get the same
        // random sequence six times in a row (this provides smoother
        // random fluctuations in brightness/color)
        if (loopCount % 6 == 0) {
            seed = random(30000);
        }

        randomSeed(seed);
    }

  // call the appropriate pattern routine based on state; these
  // routines just set the colors in the colors array
  switch (pattern)
  {
    /*case SimpleHSV:
        maxLoops = 400;
        SimpleHSV_pat();
        speed_delay(g_speed, 6);
        break;*/

    case SymSimpleHSV:        
        maxLoops = 256;
        SymSimpleHSV_pat();
        speed_delay(g_speed, 6);
        break;

    case EMS:
        maxLoops = N_LEDS * 4;
        EMS_pat();
        break;

    case Flicker:
        maxLoops = N_LEDS * 5;
        Flicker_pat();
        break;

    case RandomMarch:
        maxLoops = N_LEDS * 2;
        RandomMartch_pat();
        speed_delay(g_speed, 20);
        break;

    case Flame:
        maxLoops = N_LEDS;
        Flame_pat();
        break;

    case Matrix:
        maxLoops = N_LEDS * 3;
        Matrix_pat();
        break;

    case WarmWhiteShimmer:
      // warm white shimmer for 300 loopCounts, fading over last 70
      maxLoops = 300;
      warmWhiteShimmer(loopCount > maxLoops - 70);
      break;

    case RandomColorWalk:
      // start with alternating red and green colors that randomly walk
      // to other colors for 400 loopCounts, fading over last 80
      maxLoops = 400;
      randomColorWalk(loopCount == 0 ? 1 : 0, loopCount > maxLoops - 80);
      break;

    case TraditionalColors:
      // repeating pattern of red, green, orange, blue, magenta that
      // slowly moves for 400 loopCounts
      maxLoops = 600; // was 400
      traditionalColors();
      break;

    case ColorExplosion:
      // bursts of random color that radiate outwards from random points
      // for 630 loop counts; no burst generation for the last 70 counts
      // of every 200 count cycle or over the over final 100 counts
      // (this creates a repeating bloom/decay effect)
      maxLoops = 630;
      colorExplosion((loopCount % 200 > 130) || (loopCount > maxLoops - 100));
      break;

    case Gradient:
      // red -> white -> green -> white -> red ... gradiant that scrolls
      // across the strips for 250 counts; this pattern is overlaid with
      // waves of dimness that also scroll (at twice the speed)
      maxLoops = 250;
      gradient();
      speed_delay(g_speed, 6);  // add an extra 6ms delay to slow things down
      break;

    case BrightTwinkle:
      // random LEDs light up brightly and fade away; it is a very similar
      // algorithm to colorExplosion (just no radiating outward from the
      // LEDs that light up); as time goes on, allow progressively more
      // colors, halting generation of new twinkles for last 100 counts.
      maxLoops = 1200;
      if (loopCount < 400)
      {
        brightTwinkle(0, 1, 0);  // only white for first 400 loopCounts
      }
      else if (loopCount < 650)
      {
        brightTwinkle(0, 2, 0);  // white and red for next 250 counts
      }
      else if (loopCount < 900)
      {
        brightTwinkle(1, 2, 0);  // red, and green for next 250 counts
      }
      else
      {
        // red, green, blue, cyan, magenta, yellow for the rest of the time
        brightTwinkle(1, 6, loopCount > maxLoops - 100);
      }
      break;

    case Collision:
      // colors grow towards each other from the two ends of the strips,
      // accelerating until they collide and the whole strip flashes
      // white and fades; this repeats until the function indicates it
      // is done by returning 1, at which point we stop keeping maxLoops
      // just ahead of loopCount
      if (!collision())
      {
        maxLoops = loopCount + 2;
      }
      if (g_speed > 64) {
            speed_delay(g_speed, 3);
        }
      break;

    case RingsHSV:
      maxLoops = 400;
      RingsHSV_Loop();
      break;

    case ShootRings:
      maxLoops = 400;
      ShootRings_Loop();
      break;

    case SpinningRings:
      maxLoops = 400;
      SpinningRings_Loop(false);
      break;
      
    case SpinningRingsSym:
      maxLoops = 400;
      SpinningRings_Loop(true);
      break;


  }

  // update the LED strips with the colors in the colors array
  FastLED.show();
  loopCount++;  // increment our loop counter/timer.

    if (loopCount >= maxLoops && 1) {
        loopCount = 0;

        // AUTO ADVANCE
        if (pattern_auto_inc) {
            advance_pattern(true);
        }
    }
}


// This function applies a random walk to val by increasing or
// decreasing it by changeAmount or by leaving it unchanged.
// val is a pointer to the byte to be randomly changed.
// The new value of val will always be within [0, maxVal].
// A walk direction of 0 decreases val and a walk direction of 1
// increases val.  The directions argument specifies the number of
// possible walk directions to choose from, so when directions is 1, val
// will always decrease; when directions is 2, val will have a 50% chance
// of increasing and a 50% chance of decreasing; when directions is 3,
// val has an equal chance of increasing, decreasing, or staying the same.
void randomWalk(unsigned char *val, unsigned char maxVal, unsigned char changeAmount, unsigned char directions)
{
  unsigned char walk = random(directions);  // direction of random walk
  if (walk == 0)
  {
    // decrease val by changeAmount down to a min of 0
    if (*val >= changeAmount)
    {
      *val -= changeAmount;
    }
    else
    {
      *val = 0;
    }
  }
  else if (walk == 1)
  {
    // increase val by changeAmount up to a max of maxVal
    if (*val <= maxVal - changeAmount)
    {
      *val += changeAmount;
    }
    else
    {
      *val = maxVal;
    }
  }
}

// This function fades val by decreasing it by an amount proportional
// to its current value.  The fadeTime argument determines the
// how quickly the value fades.  The new value of val will be:
//   val = val - val*2^(-fadeTime)
// So a smaller fadeTime value leads to a quicker fade.
// If val is greater than zero, val will always be decreased by
// at least 1.
// val is a pointer to the byte to be faded.
void fade(unsigned char *val, unsigned char fadeTime)
{
  if (*val != 0)
  {
    unsigned char subAmt = *val >> fadeTime;  // val * 2^-fadeTime
    if (subAmt < 1)
      subAmt = 1;  // make sure we always decrease by at least 1
    *val -= subAmt;  // decrease value of byte pointed to by val
  }
}


// ***** PATTERN WarmWhiteShimmer *****
// This function randomly increases or decreases the brightness of the
// even red LEDs by changeAmount, capped at maxBrightness.  The green
// and blue LED values are set proportional to the red value so that
// the LED color is warm white.  Each odd LED is set to a quarter the
// brightness of the preceding even LEDs.  The dimOnly argument
// disables the random increase option when it is true, causing
// all the LEDs to get dimmer by changeAmount; this can be used for a
// fade-out effect.
void warmWhiteShimmer(unsigned char dimOnly)
{
  const unsigned char maxBrightness = 120;  // cap on LED brighness
  const unsigned char changeAmount = 2;   // size of random walk step

  for (int i = 0; i < N_LEDS; i += 2)
  {
    // randomly walk the brightness of every even LED
    randomWalk(&leds[i].red, maxBrightness, changeAmount, dimOnly ? 1 : 2);

    // warm white: red = x, green = 0.8x, blue = 0.125x
    leds[i].green = leds[i].red*4/5;  // green = 80% of red
    leds[i].blue = leds[i].red >> 3;  // blue = red/8

    // every odd LED gets set to a quarter the brighness of the preceding even LED
    if (i + 1 < N_LEDS)
    {
      leds[i+1] = CRGB(leds[i].red >> 2, leds[i].green >> 2, leds[i].blue >> 2);
    }
  }
}


// ***** PATTERN RandomColorWalk *****
// This function randomly changes the color of every seventh LED by
// randomly increasing or decreasing the red, green, and blue components
// by changeAmount (capped at maxBrightness) or leaving them unchanged.
// The two preceding and following LEDs are set to progressively dimmer
// versions of the central color.  The initializeColors argument
// determines how the colors are initialized:
//   0: randomly walk the existing colors
//   1: set the LEDs to alternating red and green segments
//   2: set the LEDs to random colors
// When true, the dimOnly argument changes the random walk into a 100%
// chance of LEDs getting dimmer by changeAmount; this can be used for
// a fade-out effect.
void randomColorWalk(unsigned char initializeColors, unsigned char dimOnly)
{
  const unsigned char maxBrightness = 180;  // cap on LED brightness
  const unsigned char changeAmount = 3;  // size of random walk step

  // pick a good starting point for our pattern so the entire strip
  // is lit well (if we pick wrong, the last four LEDs could be off)
  unsigned char start;
  switch (N_LEDS % 7)
  {
    case 0:
      start = 3;
      break;
    case 1:
      start = 0;
      break;
    case 2:
      start = 1;
      break;
    default:
      start = 2;
  }

  for (int i = start; i < N_LEDS; i+=7)
  {
    if (initializeColors == 0)
    {
      // randomly walk existing colors of every seventh LED
      // (neighboring LEDs to these will be dimmer versions of the same color)
      randomWalk(&leds[i].red, maxBrightness, changeAmount, dimOnly ? 1 : 3);
      randomWalk(&leds[i].green, maxBrightness, changeAmount, dimOnly ? 1 : 3);
      randomWalk(&leds[i].blue, maxBrightness, changeAmount, dimOnly ? 1 : 3);
    }
    else if (initializeColors == 1)
    {
      // initialize LEDs to alternating red and green
      if (i % 2)
      {
        leds[i] = CRGB(maxBrightness, 0, 0);
      }
      else
      {
        leds[i] = CRGB(0, maxBrightness, 0);
      }
    }
    else
    {
      // initialize LEDs to a string of random colors
      leds[i] = CRGB(random(maxBrightness), random(maxBrightness), random(maxBrightness));
    }

    // set neighboring LEDs to be progressively dimmer versions of the color we just set
    if (i >= 1)
    {
      leds[i-1] = CRGB(leds[i].red >> 2, leds[i].green >> 2, leds[i].blue >> 2);
    }
    if (i >= 2)
    {
      leds[i-2] = CRGB(leds[i].red >> 3, leds[i].green >> 3, leds[i].blue >> 3);
    }
    if (i + 1 < N_LEDS)
    {
      leds[i+1] = leds[i-1];
    }
    if (i + 2 < N_LEDS)
    {
      leds[i+2] = leds[i-2];
    }
  }
}


void SimpleHSV_pat()
{
    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CHSV((i + loopCount) % 255, 255, 255);
    }
}

void SymSimpleHSV_pat()
{
    for (int i = 0; i < N_LEDS/2; ++i) {
        leds[i] = CHSV((i + loopCount) % 255, 255, 255);
        leds[N_LEDS - i - 1] = leds[i];
    }
}


// ***** PATTERN TraditionalColors *****
// This function creates a repeating patern of traditional Christmas
// light colors: red, green, orange, blue, magenta.
// Every fourth LED is colored, and the pattern slowly moves by fading
// out the current set of lit LEDs while gradually brightening a new
// set shifted over one LED.
void traditionalColors()
{
  // loop counts to leave strip initially dark
  const unsigned char initialDarkCycles = 10;
  // loop counts it takes to go from full off to fully bright
  const unsigned char brighteningCycles = 20;

  if (loopCount < initialDarkCycles)  // leave strip fully off for 20 cycles
  {
    return;
  }

  // if N_LEDS is not an exact multiple of our repeating pattern size,
  // it will not wrap around properly, so we pick the closest LED count
  // that is an exact multiple of the pattern period (20) and is not smaller
  // than the actual LED count.
  unsigned int extendedLEDCount = (((N_LEDS-1)/20)+1)*20;

  for (int i = 0; i < extendedLEDCount; i++)
  {
    unsigned char brightness = (loopCount - initialDarkCycles)%brighteningCycles + 1;
    unsigned char cycle = (loopCount - initialDarkCycles)/brighteningCycles;

    // transform i into a moving idx space that translates one step per
    // brightening cycle and wraps around
    unsigned int idx = (i + cycle)%extendedLEDCount;
    if (idx < N_LEDS)  // if our transformed index exists
    {
      if (i % 4 == 0)
      {
        // if this is an LED that we are coloring, set the color based
        // on the LED and the brightness based on where we are in the
        // brightening cycle
        switch ((i/4)%5)
        {
           case 0:  // red
             leds[idx].red = 200 * brightness/brighteningCycles;
             leds[idx].green = 10 * brightness/brighteningCycles;
             leds[idx].blue = 10 * brightness/brighteningCycles;
             break;
           case 1:  // green
             leds[idx].red = 10 * brightness/brighteningCycles;
             leds[idx].green = 200 * brightness/brighteningCycles;
             leds[idx].blue = 10 * brightness/brighteningCycles;
             break;
           case 2:  // orange
             leds[idx].red = 200 * brightness/brighteningCycles;
             leds[idx].green = 120 * brightness/brighteningCycles;
             leds[idx].blue = 0 * brightness/brighteningCycles;
             break;
           case 3:  // blue
             leds[idx].red = 10 * brightness/brighteningCycles;
             leds[idx].green = 10 * brightness/brighteningCycles;
             leds[idx].blue = 200 * brightness/brighteningCycles;
             break;
           case 4:  // magenta
             leds[idx].red = 200 * brightness/brighteningCycles;
             leds[idx].green = 64 * brightness/brighteningCycles;
             leds[idx].blue = 145 * brightness/brighteningCycles;
             break;
        }
      }
      else
      {
        // fade the 3/4 of LEDs that we are not currently brightening
        fade(&leds[idx].red, 3);
        fade(&leds[idx].green, 3);
        fade(&leds[idx].blue, 3);
      }
    }
  }
}


// Helper function for adjusting the colors for the BrightTwinkle
// and ColorExplosion patterns.  Odd colors get brighter and even
// colors get dimmer.
void brightTwinkleColorAdjust(unsigned char *color)
{
  if (*color == 255)
  {
    // if reached max brightness, set to an even value to start fade
    *color = 254;
  }
  else if (*color % 2)
  {
    // if odd, approximately double the brightness
    // you should only use odd values that are of the form 2^n-1,
    // which then gets a new value of 2^(n+1)-1
    // using other odd values will break things
    *color = *color * 2 + 1;
  }
  else if (*color > 0)
  {
    fade(color, 4);
    if (*color % 2)
    {
      (*color)--;  // if faded color is odd, subtract one to keep it even
    }
  }
}


// Helper function for adjusting the colors for the ColorExplosion
// pattern.  Odd colors get brighter and even colors get dimmer.
// The propChance argument determines the likelihood that neighboring
// LEDs are put into the brightening stage when the central LED color
// is 31 (chance is: 1 - 1/(propChance+1)).  The neighboring LED colors
// are pointed to by leftColor and rightColor (it is not important that
// the leftColor LED actually be on the "left" in your setup).
void colorExplosionColorAdjust(unsigned char *color, unsigned char propChance,
 unsigned char *leftColor, unsigned char *rightColor)
{
  if (*color == 31 && random(propChance+1) != 0)
  {
    if (leftColor != 0 && *leftColor == 0)
    {
      *leftColor = 1;  // if left LED exists and color is zero, propagate
    }
    if (rightColor != 0 && *rightColor == 0)
    {
      *rightColor = 1;  // if right LED exists and color is zero, propagate
    }
  }
  brightTwinkleColorAdjust(color);
}


// ***** PATTERN ColorExplosion *****
// This function creates bursts of expanding, overlapping colors by
// randomly picking LEDs to brighten and then fade away.  As these LEDs
// brighten, they have a chance to trigger the same process in
// neighboring LEDs.  The color of the burst is randomly chosen from
// among red, green, blue, and white.  If a red burst meets a green
// burst, for example, the overlapping portion will be a shade of yellow
// or orange.
// When true, the noNewBursts argument changes prevents the generation
// of new bursts; this can be used for a fade-out effect.
// This function uses a very similar algorithm to the BrightTwinkle
// pattern.  The main difference is that the random twinkling LEDs of
// the BrightTwinkle pattern do not propagate to neighboring LEDs.
void colorExplosion(unsigned char noNewBursts)
{
  // adjust the colors of the first LED
  colorExplosionColorAdjust(&leds[0].red, 9, (unsigned char*)0, &leds[1].red);
  colorExplosionColorAdjust(&leds[0].green, 9, (unsigned char*)0, &leds[1].green);
  colorExplosionColorAdjust(&leds[0].blue, 9, (unsigned char*)0, &leds[1].blue);

  for (int i = 1; i < N_LEDS - 1; i++)
  {
    // adjust the colors of second through second-to-last LEDs
    colorExplosionColorAdjust(&leds[i].red, 9, &leds[i-1].red, &leds[i+1].red);
    colorExplosionColorAdjust(&leds[i].green, 9, &leds[i-1].green, &leds[i+1].green);
    colorExplosionColorAdjust(&leds[i].blue, 9, &leds[i-1].blue, &leds[i+1].blue);
  }

  // adjust the colors of the last LED
  colorExplosionColorAdjust(&leds[N_LEDS-1].red, 9, &leds[N_LEDS-2].red, (unsigned char*)0);
  colorExplosionColorAdjust(&leds[N_LEDS-1].green, 9, &leds[N_LEDS-2].green, (unsigned char*)0);
  colorExplosionColorAdjust(&leds[N_LEDS-1].blue, 9, &leds[N_LEDS-2].blue, (unsigned char*)0);

  if (!noNewBursts)
  {
    // if we are generating new bursts, randomly pick one new LED
    // to light up
    for (int i = 0; i < 1; i++)
    {
      int j = random(N_LEDS);  // randomly pick an LED

      switch(random(7))  // randomly pick a color
      {
        // 2/7 chance we will spawn a red burst here (if LED has no red component)
        case 0:
        case 1:
          if (leds[j].red == 0)
          {
            leds[j].red = 1;
          }
          break;

        // 2/7 chance we will spawn a green burst here (if LED has no green component)
        case 2:
        case 3:
          if (leds[j].green == 0)
          {
            leds[j].green = 1;
          }
          break;

        // 2/7 chance we will spawn a white burst here (if LED is all off)
        case 4:
        case 5:
          if ((leds[j].red == 0) && (leds[j].green == 0) && (leds[j].blue == 0))
          {
            leds[j] = CRGB(1, 1, 1);
          }
          break;

        // 1/7 chance we will spawn a blue burst here (if LED has no blue component)
        case 6:
          if (leds[j].blue == 0)
          {
            leds[j].blue = 1;
          }
          break;

        default:
          break;
      }
    }
  }
}


// ***** PATTERN Gradient *****
// This function creates a scrolling color gradient that smoothly
// transforms from red to white to green back to white back to red.
// This pattern is overlaid with waves of brightness and dimness that
// scroll at twice the speed of the color gradient.
void gradient()
{
  unsigned int j = 0;

  // populate colors array with full-brightness gradient colors
  // (since the array indices are a function of loopCount, the gradient
  // colors scroll over time)
  while (j < N_LEDS)
  {
    // transition from red to green over 8 LEDs
    for (int i = 0; i < 8; i++)
    {
      if (j >= N_LEDS){ break; }
      leds[(loopCount/2 + j + N_LEDS)%N_LEDS] = CRGB(160 - 20*i, 20*i, (160 - 20*i)*20*i/160);
      j++;
    }
    // transition from green to red over 8 LEDs
    for (int i = 0; i < 8; i++)
    {
      if (j >= N_LEDS){ break; }
      leds[(loopCount/2 + j + N_LEDS)%N_LEDS] = CRGB(20*i, 160 - 20*i, (160 - 20*i)*20*i/160);
      j++;
    }
  }

  // modify the colors array to overlay the waves of dimness
  // (since the array indices are a function of loopCount, the waves
  // of dimness scroll over time)
  const unsigned char fullDarkLEDs = 10;  // number of LEDs to leave fully off
  const unsigned char fullBrightLEDs = 5;  // number of LEDs to leave fully bright
  const unsigned char cyclePeriod = 14 + fullDarkLEDs + fullBrightLEDs;

  // if N_LEDS is not an exact multiple of our repeating pattern size,
  // it will not wrap around properly, so we pick the closest LED count
  // that is an exact multiple of the pattern period (cyclePeriod) and is not
  // smaller than the actual LED count.
  unsigned int extendedLEDCount = (((N_LEDS-1)/cyclePeriod)+1)*cyclePeriod;

  j = 0;
  while (j < extendedLEDCount)
  {
    unsigned int idx;

    // progressively dim the LEDs
    for (int i = 1; i < 8; i++)
    {
      idx = (j + loopCount) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= N_LEDS){ continue; }

      leds[idx].red >>= i;
      leds[idx].green >>= i;
      leds[idx].blue >>= i;
    }

    // turn off these LEDs
    for (int i = 0; i < fullDarkLEDs; i++)
    {
      idx = (j + loopCount) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= N_LEDS){ continue; }

      leds[idx].red = 0;
      leds[idx].green = 0;
      leds[idx].blue = 0;
    }

    // progressively bring these LEDs back
    for (int i = 0; i < 7; i++)
    {
      idx = (j + loopCount) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= N_LEDS){ continue; }

      leds[idx].red >>= (7 - i);
      leds[idx].green >>= (7 - i);
      leds[idx].blue >>= (7 - i);
    }

    // skip over these LEDs to leave them at full brightness
    j += fullBrightLEDs;
  }
}


// ***** PATTERN BrightTwinkle *****
// This function creates a sparkling/twinkling effect by randomly
// picking LEDs to brighten and then fade away.  Possible colors are:
//   white, red, green, blue, yellow, cyan, and magenta
// numColors is the number of colors to generate, and minColor
// indicates the starting point (white is 0, red is 1, ..., and
// magenta is 6), so colors generated are all of those from minColor
// to minColor+numColors-1.  For example, calling brightTwinkle(2, 2, 0)
// will produce green and blue twinkles only.
// When true, the noNewBursts argument changes prevents the generation
// of new twinkles; this can be used for a fade-out effect.
// This function uses a very similar algorithm to the ColorExplosion
// pattern.  The main difference is that the random twinkling LEDs of
// this BrightTwinkle pattern do not propagate to neighboring LEDs.
void brightTwinkle(unsigned char minColor, unsigned char numColors, unsigned char noNewBursts)
{
  // Note: the colors themselves are used to encode additional state
  // information.  If the color is one less than a power of two
  // (but not 255), the color will get approximately twice as bright.
  // If the color is even, it will fade.  The sequence goes as follows:
  // * Randomly pick an LED.
  // * Set the color(s) you want to flash to 1.
  // * It will automatically grow through 3, 7, 15, 31, 63, 127, 255.
  // * When it reaches 255, it gets set to 254, which starts the fade
  //   (the fade process always keeps the color even).
  for (int i = 0; i < N_LEDS; i++)
  {
    brightTwinkleColorAdjust(&leds[i].red);
    brightTwinkleColorAdjust(&leds[i].green);
    brightTwinkleColorAdjust(&leds[i].blue);
  }

  if (!noNewBursts)
  {
    // if we are generating new twinkles, randomly pick four new LEDs
    // to light up
    for (int i = 0; i < 4; i++)
    {
      int j = random(N_LEDS);
      if (leds[j].red == 0 && leds[j].green == 0 && leds[j].blue == 0)
      {
        // if the LED we picked is not already lit, pick a random
        // color for it and seed it so that it will start getting
        // brighter in that color
        switch (random(numColors) + minColor)
        {
          case 0:
            leds[j] = CRGB(1, 1, 1);  // white
            break;
          case 1:
            leds[j] = CRGB(1, 0, 0);  // red
            break;
          case 2:
            leds[j] = CRGB(0, 1, 0);  // green
            break;
          case 3:
            leds[j] = CRGB(0, 0, 1);  // blue
            break;
          case 4:
            leds[j] = CRGB(1, 1, 0);  // yellow
            break;
          case 5:
            leds[j] = CRGB(0, 1, 1);  // cyan
            break;
          case 6:
            leds[j] = CRGB(1, 0, 1);  // magenta
            break;
          default:
            leds[j] = CRGB(1, 1, 1);  // white
        }
      }
    }
  }
}


// ***** PATTERN Collision *****
// This function spawns streams of color from each end of the strip
// that collide, at which point the entire strip flashes bright white
// briefly and then fades.  Unlike the other patterns, this function
// maintains a lot of complicated state data and tells the main loop
// when it is done by returning 1 (a return value of 0 means it is
// still in progress).
unsigned char collision()
{
  const unsigned char maxBrightness = 180;  // max brightness for the colors
  const unsigned char numCollisions = 5;  // # of collisions before pattern ends
  static unsigned char state = 0;  // pattern state
  static unsigned int count = 0;  // counter used by pattern

  if (loopCount == 0)
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

#define TOP_INDEX (N_LEDS/2)
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {iN = ( i + TOP_INDEX ) % N_LEDS; }
  return iN;
}

int thishue = 0;
void EMS_pat() {                  //-m8-EMERGENCY LIGHTS (TWO COLOR SOLID)
  int loopCountR = (loopCount % N_LEDS);
  int loopCountB = antipodal_index(loopCountR);
  int thathue = (thishue + 160) % 255;
  leds[loopCountR] = CHSV(thishue, 255, 255);
  leds[loopCountB] = CHSV(thathue, 255, 255);  

  if ((loopCount % 3) == 0) ++thishue;

  speed_delay(g_speed, 25);
}

void Flicker_pat() {
  int random_bright = random(0,255);
  int random_delay = random(10,100);
  int random_bool = random(0,random_bright);
  if (random_bool < 10) {
    for(int i = 0 ; i < N_LEDS; i++ ) {
      leds[i] = CHSV(160, 50, random_bright);
    }
    speed_delay(g_speed, random_delay / 2);
  }

}

int adjacent_ccw(int i) {
  int r;
  if (i > 0) {r = i - 1;}
  else {r = N_LEDS - 1;}
  return r;
}


void RandomMartch_pat()
{
  int iCCW;
  copy_led_array();
  leds[0] = CHSV(random(0,255), 255, 255);
  for(int idex = 1; idex < N_LEDS ; idex++ ) {
    iCCW = adjacent_ccw(idex);
    leds[idex].r = ledsX[iCCW][0];
    leds[idex].g = ledsX[iCCW][1];
    leds[idex].b = ledsX[iCCW][2];
  }
  if (loopCount > N_LEDS) {
    for (int i = 0; i < loopCount - N_LEDS; ++i) {
        if (i >= N_LEDS) break;
        leds[i] = CRGB(0,0,0);
    }
  }

  LEDS.show();  
}

#define EVENODD (N_LEDS % 2)
int horizontal_index(int i) {
  //-ONLY WORKS WITH INDEX < TOPINDEX
  if (i == 0) {return 0;}
  if (i == TOP_INDEX && EVENODD == 1) {return TOP_INDEX + 1;}
  if (i == TOP_INDEX && EVENODD == 0) {return TOP_INDEX;}
  return N_LEDS - i;  
}

void Flame_pat() {
  int ghue = (0 + 80) % 255;
  int bhue = (0 + 160) % 255;
  int N3  = int(N_LEDS/3);
  int N6  = int(N_LEDS/6);  
  int N12 = int(N_LEDS/12);  
  int idex = loopCount;
  int thissat = 200;
  for(int i = 0; i < N3; i++ ) {
    int j0 = (idex + i + N_LEDS - N12) % N_LEDS;
    int j1 = (j0+N3) % N_LEDS;
    int j2 = (j1+N3) % N_LEDS;    
    leds[j0] = CHSV(0 , thissat, 255);
    leds[j1] = CHSV(ghue, thissat, 255);
    leds[j2] = CHSV(bhue, thissat, 255);    
  }
  speed_delay(g_speed, 10);
}

void Matrix_pat() {
    int thishue = 95;
    int thissat = 255;

  int rand = random(0, 100);
  if (rand > 90) {
    leds[0] = CHSV(thishue, thissat, 255);
  }
  else {leds[0] = CHSV(thishue, thissat, 0);}
  copy_led_array();
    for(int i = 1; i < N_LEDS; i++ ) {
    leds[i].r = ledsX[i-1][0];
    leds[i].g = ledsX[i-1][1];
    leds[i].b = ledsX[i-1][2];    
  }
  speed_delay(g_speed, 35);
}


void efx_blink(int h, int repeats) {
    for (int cnt = 0; cnt < repeats; ++cnt) {
        for (int v = 50; v < 255; v += 3) {
            for (int i = 0; i < N_LEDS; ++i) {
                leds[i] = CHSV(h, 255, v);
            }
            LEDS.show();
        }
        for (int v = 255; v > 50; v -= 3) {
            for (int i = 0; i < N_LEDS; ++i) {
                leds[i] = CHSV(h, 255, v);
            }
            LEDS.show();
        }
    }
}


class BasePattern2 {
public:
    const LedRange * ranges;
    unsigned char n_ranges;
    unsigned short n_leds;

    BasePattern2(const LedRange * ranges, unsigned char n_ranges) {
        this->ranges = ranges;
        this->n_ranges = n_ranges;
        this->n_leds = 0;

        for (unsigned char i = 0; i < n_ranges; ++i) {
            this->n_leds += ranges[i].end - ranges[i].start + 1;
        }
    }

    CRGB * pixel(unsigned char idx) {
        unsigned char cur_idx = 0;
        unsigned char n_leds;
        for (unsigned char range = 0; range < this->n_ranges; ++range) {
            n_leds = this->ranges[range].end - this->ranges[range].start + 1;
            
            if ((cur_idx + n_leds) <= idx) {
                cur_idx += n_leds;
            } else {
                return &(leds[
                    this->ranges[range].start + (idx - cur_idx)
                ]);
            }
        }

        while(1) {
            Serial.println("fuck");
        }
    }

    void setPixel(unsigned char idx, CRGB val) {
        CRGB * pix = this->pixel(idx);
        pix->r = val.r;
        pix->g = val.g;
        pix->b = val.b;
    }    
    void setRange(unsigned char range, CRGB val) {
        for (unsigned char i = ranges[range].start; i <= ranges[range].end; ++i )
        {
            leds[i] = val;
        }
    }

    void clearRange(unsigned char range) {
        memset(&(leds[ranges[range].start]), 0, (ranges[range].end - ranges[range].start + 1) * sizeof(CRGB));
    }

};

class HSVPattern : public BasePattern2 {
public:
    unsigned char start_hsv;
    unsigned char num_hsvs;

    HSVPattern(const LedRange * ranges, unsigned char n_ranges, unsigned char start_hsv, unsigned char num_hsvs) : BasePattern2(ranges, n_ranges) {
        this->start_hsv = start_hsv;
        this->num_hsvs = num_hsvs;
    }

    void loop() {
        for (int i = 0; i < n_ranges / 2; ++i) {
            CRGB rgb = CHSV(
                    start_hsv +  (i + loopCount) % num_hsvs,
                255, 255);
            setRange(i, rgb);
            setRange((n_ranges / 2) + i, rgb);
        }
    }
};

class RingsPatterns : public BasePattern2 {
public:
    bool speedup;
    int speed_cnt;

   RingsPatterns(const LedRange * ranges, unsigned char n_ranges) : BasePattern2(ranges, n_ranges) {
    speedup = false;
    speed_cnt = 0;

    }

    void loop_shoot() {
        int half = n_ranges / 2;
        for (int i = 0; i < half; ++i) {
            int cur_range = loopCount % half;
            CRGB rgb;
            if (cur_range == i) {
                rgb = CHSV(loopCount % 255, 255, 255);
                setRange(i, rgb);
                setRange(half + i, rgb);
            } else {
                clearRange(i);
                clearRange(half + i);
            }
        }
    }

    void loop_spinning(bool sym) {
        for (int i = 0; i < n_ranges; ++i) {
            clearRange(i);

        
            int range_leds = ranges[i].end - ranges[i].start + 1;
            int led_idx = map(loopCount % 30, 0, 30, 0, range_leds);

            if (sym && (i >= (n_ranges/2))) {
                led_idx = range_leds - led_idx - 1;
            }

 
            for (int j = 0; j < 3; ++j) {
                leds[ranges[i].start + ((led_idx + j) % range_leds)] = CHSV((loopCount + (j*2)) % 255, 255, 255);
            }
        }
    }

    void delay() {
        int d;
        if (loopCount == 0) {
            speedup = random(100) > 50;
            Serial.println(speedup);
        //    speedup = true;
            speed_cnt = 0;
        }

        if (speedup) {
            speed_cnt += 1;
            if (speed_cnt < 160) {
                d = 20 - (speed_cnt / 16);
            } else {
                d = 20 - (speed_cnt / 20);
            }
            if (0 > d) d = 0;
        } else {
            d = 20;
        }
        
        speed_delay(g_speed, d);
    }
};


HSVPattern RingsHSV_pattern(rings, n_rings, 230, 90);
HSVPattern LinesHSV_pattern(lines, n_lines, 110, 100);

RingsPatterns Rings_pattern(rings, n_rings);


void RingsHSV_Loop() {
    RingsHSV_pattern.loop();
    LinesHSV_pattern.loop();
    speed_delay(g_speed, 20);
}

void ShootRings_Loop() {
    Rings_pattern.loop_shoot();
    Rings_pattern.delay();
}

void SpinningRings_Loop(bool sym) {
    Rings_pattern.loop_spinning(sym);
    Rings_pattern.delay();
}


class MICBarPattern : public BasePattern2 {
public:
    MICBarPattern(const LedRange * ranges, unsigned char n_ranges) : BasePattern2(ranges, n_ranges) {
    }

    void loop() {
        int x = analogRead(3) >> 3;
        static int last_x = 0;
        static unsigned char h = 0;
        
        if (x >= last_x) {
            last_x = x;
        } else {
//            last_x--;
              last_x = (last_x*7 + x) / 8;
        }
        //Serial.println(last_x);
//        delay(5);

        int max_rings = n_ranges / 2;
        int scaler = (g_speed >> 2);

        int lit_rings = map(last_x * scaler, 0, 127, 0, max_rings);
       
//        Serial.print(last_x);        Serial.print("            ");Serial.print(lit_rings);Serial.print("       ");Serial.println(scaler);
        memset(leds, 0, sizeof(leds));
        for (int i = 0; i < max_rings; ++i) {
            if (i < lit_rings) {
                CRGB c = CHSV(
                    //map(last_x * scaler, 0, 512, 96, 0),
                    (unsigned char) (map(i, 0, max_rings, 90, -20) + random(20)),
                    255,
                    255
                );
                setRange(n_ranges - i - 1, c);
                setRange(n_ranges - i - 1 - max_rings, c);
            }
        }
        FastLED.show();

    }
};


#define MIC_PIN   A3  // Microphone is attached to this analog pin
#define DC_OFFSET  0  // DC offset in mic signal - if unusure, leave 0
#define NOISE     20  // Noise/hum/interference in mic signal
#define SAMPLES   40  // Length of buffer for dynamic level adjustment
//#define TOP       (N_LEDS - 2) // Allow dot to go slightly off scale
#define PEAK_FALL 5  // Rate of peak falling dot

class MICVUPattern : public BasePattern2 {
public:
    byte peak, dotCount, volCount;
    int vol[SAMPLES];
    int lvl, minLvlAvg, maxLvlAvg;
    int TOP;


    MICVUPattern(const LedRange * ranges, unsigned char n_ranges) : BasePattern2(ranges, n_ranges)
    {
        peak      = 0;      // Used for falling dot
        dotCount  = 0;      // Frame counter for delaying dot-falling speed
        volCount  = 0;      // Frame counter for storing past volume data
    
        memset(vol, 0, sizeof(vol));
        lvl       = 20;      // Current "dampened" audio level
        minLvlAvg = 0;      // For dynamic adjustment of graph low & high
        maxLvlAvg = 512;

        TOP = n_ranges / 2;
    }

    void loop() {
        uint8_t  i;
        uint16_t minLvl, maxLvl;
        int      n, height;

        memset(leds, 0, sizeof(leds));

        n   = analogRead(A3);                  // Raw reading from mic
//        n   = abs(n - 512 - DC_OFFSET); // Center on zero
        n   = (n <= NOISE) ? 0 : (n - NOISE);             // Remove noise/hum
        lvl = ((lvl * 7) + n) >> 3;    // "Dampened" reading (else looks twitchy)

        // Calculate bar height based on dynamic min/max levels (fixed point):
        height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

        if(height < 0L)       height = 0;      // Clip output
        else if(height > TOP) height = TOP;
        if(height > peak)     peak   = height; // Keep 'peak' dot at top


        // Color pixels based on rainbow gradient
        for(i=0; i<TOP; i++) {
            if(i >= height) {
//                setRange(i, CRGB::Black);
            }
            else Wheel(i, map(i,0,TOP,30,150));   
        }

        // Draw peak dot 
        if(peak > 0 && peak <= TOP-1) Wheel(peak, map(peak,0,TOP,30,150));

        FastLED.show(); // Update strip

        // Every few frames, make the peak pixel drop by 1:
        if(++dotCount >= PEAK_FALL) { //fall rate
         
          if(peak > 0) peak--;
          dotCount = 0;
        }


        vol[volCount] = n;                      // Save sample for dynamic leveling
        if(++volCount >= SAMPLES) volCount = 0; // Advance/rollover sample counter

        // Get volume range of prior frames
        minLvl = maxLvl = vol[0];
        for(i=1; i<SAMPLES; i++) {
        if(vol[i] < minLvl)      minLvl = vol[i];
        else if(vol[i] > maxLvl) maxLvl = vol[i];
        }
        // minLvl and maxLvl indicate the volume range over prior frames, used
        // for vertically scaling the output graph (so it looks interesting
        // regardless of volume level).  If they're too close together though
        // (e.g. at very low volume levels) the graph becomes super coarse
        // and 'jumpy'...so keep some minimum distance between them (this
        // also lets the graph go to zero when no sound is playing):
        if((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
        minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6; // Dampen min/max levels
        maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6; // (fake rolling average)
    }

    // Input a value 0 to 255 to get a color value.
    // The colors are a transition r - g - b - back to r.
    void Wheel(int i, byte WheelPos) {
        CRGB c;
      if(WheelPos < 85) {
       c = CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
      } else if(WheelPos < 170) {
       WheelPos -= 85;
        c = CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
      } else {
       WheelPos -= 170;
      c  = CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
      }

//      setRange(TOP - i - 1 , c);
      setRange(n_ranges - i - 1, c);
      setRange(TOP - i - 1, c);
    }

};
MICBarPattern micBar1(rings, n_rings);
MICVUPattern micBar2(rings, n_rings);
void micbar1loop() {
    micBar1.loop();
}
void micbar2loop() {
    micBar2.loop();
}



