#include <MemoryFree.h>
#include "FastSPI_LED2.h"
#include <EEPROM.h>
#include <Encoder.h>
#include "BlinkyCommon.h"
#include "BlinkyFaderPattern.h"
#include "BlinkyMiscPatterns.h"
#include "BlinkyAudioPatterns.h"


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

int PANTS_VERSION = 2;
int N_LEDS;


//
unsigned char g_brightness = 16;
unsigned char g_speed = 10;
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
HSVPattern RingsHSV_pattern(rings, n_rings, 230, 90);
HSVPattern LinesHSV_pattern(lines, n_lines, 110, 100);
RingsPatterns Rings_pattern(rings, n_rings);

MICBarPattern micBar1(rings, n_rings);
MICVUPattern micBar2(rings, n_rings);

inline void RingsHSV_Loop() { RingsHSV_pattern.loop(); LinesHSV_pattern.loop(); speed_delay(0, 100); }
inline void ShootRings_Loop() { Rings_pattern.loop_shoot(); Rings_pattern.delay(); }
inline void SpinningRings_Loop1() { Rings_pattern.loop_spinning(false); Rings_pattern.delay(); }
inline void SpinningRings_Loop2() { Rings_pattern.loop_spinning(true); Rings_pattern.delay(); }
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
inline void micBar1_loop() { micBar1.loop(); }
inline void micBar2_loop() { micBar2.loop(); }




const PatternInstance PantsV1_PatternInstances[] = {
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
};

const PatternInstance PantsV2_PatternInstances[] = {
    {EMS_pat, 5000},
    {SpinningRings_Loop2, 5000},
    {SpinningRings_Loop1, 5000},
    {ShootRings_Loop, 5000},
    {RingsHSV_Loop, 5000},
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

const PatternInstance Mic_PatternInstances[] = {
//    {micBar1_loop, 0xffffffff},
//    {micBar2_loop, 0xffffffff},
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

void set_default_patterns() {
    if (PANTS_VERSION == 1) {
        switch_patterns(PantsV1_PatternInstances, arr_len(PantsV1_PatternInstances));
    } else {
        switch_patterns(PantsV2_PatternInstances, arr_len(PantsV2_PatternInstances));
    }
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
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, N_LEDS);
    }    
    else if (PANTS_VERSION == 2)
    {
        N_LEDS = 192;
        FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, N_LEDS);        
    }
    leds2 = &(leds[N_LEDS/2]);

    FastLED.setBrightness(128);
    pinMode(DATA_PIN, OUTPUT);

    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
    
    set_default_patterns();
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
            /* long press, toggle mic mode on / off */
            if (g_patterns == Mic_PatternInstances) {
                efx_blink(60, 2);
                set_default_patterns();
            } else {
                efx_blink(90, 2);
                switch_patterns(Mic_PatternInstances, arr_len(Mic_PatternInstances));
            }
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
            enc1_moved_without_btn(new_pos > enc_pos);
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


inline void loop_speed() {
    g_speed = (analogRead(SPEED_PIN) >> 3);
}

inline void loop_pattern() {       
    g_patterns[g_pattern].loop();

    ++g_step;
    FastLED.show();

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
