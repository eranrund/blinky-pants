#include <MemoryFree.h>
#include "FastSPI_LED2.h"
#include <EEPROM.h>
#include <Encoder.h>
#include "BlinkyCommon.h"
#include "BlinkyFaderPattern.h"
#include "BlinkyMiscPatterns.h"


////////////////////////////////////////////////////////////////////////////////
// Config
////////////////////////////////////////////////////////////////////////////////

#define DATA_PIN 4
#define MIC_SW 10

#define MAX_LEDS 67


CRGB leds[MAX_LEDS];
CRGB ledsX[MAX_LEDS];
CRGB * leds2;

int N_LEDS = MAX_LEDS;


//
unsigned char g_brightness = 64;
unsigned char g_speed = 64;
unsigned char g_pattern = 0;
unsigned long g_pattern_duration = 0;
unsigned long g_pattern_last_switch_at = 0;
unsigned char g_num_patterns = 0;
unsigned int g_step = 0;
 


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
inline void SymSimpleHSV_pat();
inline void EMS_pat();
inline void Flicker_pat();
inline void RandomMartch_pat();
inline void Flame_pat();
inline void Matrix_pat();

const PatternInstance Pants_PatternInstances[] = {
    {collision, 5000},
    {brightTwinkle, 5000},
    {gradient, 5000},
    {colorExplosion, 5000},
    {traditionalColors, 5000},
    {warmWhiteShimmer, 5000},
    {Matrix_pat, 5000},
    {Flame_pat, 5000},
    {RandomMartch_pat, 5000},            
    {Flicker_pat, 5000},
    {EMS_pat, 5000},
    {SymSimpleHSV_pat, 5000},
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

#define PANTS_N_PATTERNS arr_len(Pants_PatternInstances)

// "Manager"
const PatternInstance * g_patterns;

void switch_patterns(const PatternInstance * patterns, int n_patterns) {
    g_patterns = patterns;
    g_num_patterns = n_patterns;
    goto_pattern(0);
}

void goto_pattern(unsigned char p) {
    g_pattern = p;
    g_step = 0;

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
        g_pattern = ((unsigned char)(g_pattern + (g_num_patterns - 1))) % g_num_patterns;
    }
    
    goto_pattern(g_pattern);
}


////////////////////////////////////////////////////////////////////////////////
// setup
////////////////////////////////////////////////////////////////////////////////

void set_default_patterns() {
    switch_patterns(Pants_PatternInstances, PANTS_N_PATTERNS);
}


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


    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, N_LEDS);
    leds2 = &(leds[N_LEDS/2]);

    FastLED.setBrightness(128);
    pinMode(DATA_PIN, OUTPUT);

    pinMode(MIC_SW, INPUT_PULLUP);
    delay(30);

    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
    
    set_default_patterns();
}

////////////////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////////////////


inline void loop_serial() {
    unsigned char ch;
    while (Serial.available()) {
        ch = Serial.read();
        switch (ch) {
            case 'a':
                // TODO pattern_auto_inc = !pattern_auto_inc;

                /* fallthrough */                
            case '?':
                Serial.print("Pattern: ");   
/* TODO                if (pattern_auto_inc) {
                   Serial.print("autoinc ");
                } */
                Serial.println(g_pattern);

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


inline void loop_pattern() {       
    g_patterns[g_pattern].loop();

    ++g_step;
    FastLED.show();

    /*if (millis() > (g_pattern_last_switch_at + g_pattern_duration)) {
       advance_pattern(true);
    }*/
}

void loop() {
    loop_serial();
    loop_pattern();

    if (0 == digitalRead(MIC_SW)) {
        delay(50);
        if (0 == digitalRead(MIC_SW)) {
            advance_pattern(true);
            delay(100);
        }
    }
}

void efx_blink(int h, int repeats) {
    for (int cnt = 0; cnt < repeats; ++cnt) {
        for (int v = 50; v < 255; v += 3) {
            for (int i = 0; i < N_LEDS; ++i) {
                leds[i] = CHSV(h, 255, v);
            }
            FastLED.show();
        }
        for (int v = 255; v > 50; v -= 3) {
            for (int i = 0; i < N_LEDS; ++i) {
                leds[i] = CHSV(h, 255, v);
            }
            FastLED.show();
        }
    }
}
