#define ARM_MATH_CM4
#include <arm_math.h>

#include "FastLED.h"
#include <EEPROM.h>
#include "BlinkyCommon.h"
#include "BlinkyFaderPattern.h"
#include "BlinkyMiscPatterns.h"
//#include "BlinkyAudioPatterns.h"


////////////////////////////////////////////////////////////////////////////////
// Config
////////////////////////////////////////////////////////////////////////////////

// strips
#define S1_LEN 34
#define S2_LEN 38
#define S3_LEN 39 
#define S4_LEN 39
#define S5_LEN 38
#define S6_LEN 34

#define S1_OFFSET 0
#define S2_OFFSET (S1_OFFSET + S1_LEN)
#define S3_OFFSET (S2_OFFSET + S2_LEN)
#define S4_OFFSET (S3_OFFSET + S3_LEN)
#define S5_OFFSET (S4_OFFSET + S4_LEN)
#define S6_OFFSET (S5_OFFSET + S5_LEN)

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

#define BRIGHTNESS_PIN A8
#define SPEED_PIN A9

#define MAX_LEDS 250


CRGB leds[MAX_LEDS];
CRGB ledsX[MAX_LEDS];
CRGB * leds2;

unsigned int N_LEDS;


//
unsigned char g_brightness = 16;
unsigned char g_speed = 128;
unsigned char g_pattern = 0;
unsigned long g_pattern_duration = 0;
unsigned long g_pattern_last_switch_at = 0;
unsigned char g_num_patterns = 0;
unsigned int g_step = 0;
bool g_ext_ctrl_board_enable = false;
 


//
unsigned int seed = 0;



int SAMPLE_RATE_HZ = 9000;             // Sample rate of the audio in hertz.
float SPECTRUM_MIN_DB = 30.0;          // Audio intensity (in decibels) that maps to low LED brightness.
float SPECTRUM_MAX_DB = 60.0;          // Audio intensity (in decibels) that maps to high LED brightness.
const int FFT_SIZE = 256;              // Size of the FFT.  Realistically can only be at most 256 
                                       // without running out of memory for buffers and other state.
const int AUDIO_INPUT_PIN = 17;        // Input ADC pin for audio data.
const int ANALOG_READ_RESOLUTION = 10; // Bits of resolution for the ADC.
const int ANALOG_READ_AVERAGING = 16;  // Number of samples to average with each ADC reading.
const int NEO_PIXEL_COUNT = S1_LEN;         // Number of neo pixels.  You should be able to increase this without
                                       // any other changes to the program.

IntervalTimer samplingTimer;
float samples[FFT_SIZE*2];
float magnitudes[FFT_SIZE];
int sampleCounter = 0;
float frequencyWindow[NEO_PIXEL_COUNT+1];
float hues[NEO_PIXEL_COUNT];
void spectrumSetup();
void spectrumLoop();
void samplingBegin();
boolean samplingIsDone() ;



////////////////////////////////////////////////////////////////////////////////
// Pattern Instances
////////////////////////////////////////////////////////////////////////////////
BlinkyFaderPattern1 FaderPattern1;
HSVPattern RingsHSV_pattern(rings, n_rings, 230, 90);
HSVPattern LinesHSV_pattern(lines, n_lines, 110, 100);
RingsPatterns Rings_pattern(rings, n_rings);

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
inline void FaderPattern1_loop2_5() { 
    FaderPattern1.loop2(true, false, true, S1_OFFSET, S1_LEN, 0);
    FaderPattern1.loop2(true, false, true, S6_OFFSET, S6_LEN, 0);

    FaderPattern1.loop2(true, false, true, S2_OFFSET, S2_LEN, 85);
    FaderPattern1.loop2(true, false, true, S4_OFFSET, S4_LEN, 85);

    FaderPattern1.loop2(true, false, true, S3_OFFSET, S3_LEN, 160);
    FaderPattern1.loop2(true, false, true, S5_OFFSET, S5_LEN, 160);

    FaderPattern1.inc();
    FastLED.delay(4);
}
inline void FaderPattern1_loop2_6() { FaderPattern1.loop2(false, true, true); }
inline void FaderPattern1_loop2_7() { FaderPattern1.loop2(true, true, true); }
inline void FaderPattern1_loop3() { FaderPattern1.loop3(); }
inline void SymSimpleHSV_pat();
inline void EMS_pat();
inline void Flicker_pat();
inline void RandomMartch_pat();
inline void Flame_pat();
inline void Matrix_pat();

inline void Fire_pat() 
{
    static byte h1[S1_LEN];
    static byte h2[S2_LEN];
    static byte h3[S3_LEN];
    static byte h4[S4_LEN];
    static byte h5[S5_LEN];
    static byte h6[S6_LEN];
    random16_add_entropy( random());

    fire_pattern(h1, &(leds[S1_OFFSET]), S1_LEN);
    fire_pattern(h2, &(leds[S2_OFFSET]), S2_LEN);
    fire_pattern(h3, &(leds[S3_OFFSET]), S3_LEN);
    fire_pattern(h4, &(leds[S4_OFFSET]), S4_LEN);
    fire_pattern(h5, &(leds[S5_OFFSET]), S5_LEN);
    fire_pattern(h6, &(leds[S6_OFFSET]), S6_LEN);


    FastLED.delay(max(0, map(g_speed, 0, 0xff, 0, 150)));
    FastLED.show();
}


const PatternInstance Pants_PatternInstances[] = {
///?    {collision, 6000},
    {Fire_pat, 10000},
    {brightTwinkle, 30000},
    {gradient, 7000},
    {colorExplosion, 5000},
    {traditionalColors, 5000},
//    {warmWhiteShimmer, 7000},
    {Matrix_pat, 15000},
    {Flame_pat, 7000},
    {RandomMartch_pat, 19000},            
    {Flicker_pat, 10000},
    {EMS_pat, 20000},
    {SymSimpleHSV_pat, 10000},
    {FaderPattern1_loop2_5, 5375},
 // ?   {FaderPattern1_loop1, 5000},
  //  {FaderPattern1_loop2_0, 5375},
  //  {FaderPattern1_loop2_1, 5375},
  //  {FaderPattern1_loop2_2, 5375},
  //  {FaderPattern1_loop2_3, 5375},
  //  {FaderPattern1_loop2_4, 5375},
  //  {FaderPattern1_loop2_6, 5375},
   // {FaderPattern1_loop2_7, 5375},
//    {FaderPattern1_loop3, 16000},
    //{SpinningRings_Loop2, 17000},
    //{SpinningRings_Loop1, 15000},
    //{ShootRings_Loop, 10000},
    //{RingsHSV_Loop, 10000},
};

// "Manager"
const PatternInstance * g_patterns;

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

void switch_patterns(const PatternInstance * patterns, int n_patterns) {
    g_patterns = patterns;
    g_num_patterns = n_patterns;
    goto_pattern(0);
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
    switch_patterns(Pants_PatternInstances, arr_len(Pants_PatternInstances));
}


// initialization stuff
void setup()
{
    Serial.begin(9600);
    Serial.println("OK");

    delay(1000);
    for (int i = 0; i < 8; i++)
    {
        seed += analogRead(i);
    }
    seed += EEPROM.read(0);  // get part of the seed from EEPROM
    randomSeed(seed);
    EEPROM.write(0, random(256));


    N_LEDS = S6_OFFSET + S6_LEN;
    FastLED.addLeds<WS2812B, 2, GRB>(&(leds[S1_OFFSET]), S1_LEN);
    FastLED.addLeds<WS2812B, 14, GRB>(&(leds[S2_OFFSET]), S2_LEN);
    FastLED.addLeds<WS2812B, 7, GRB>(&(leds[S3_OFFSET]), S3_LEN);
    FastLED.addLeds<WS2812B, 8, GRB>(&(leds[S4_OFFSET]), S4_LEN);
    FastLED.addLeds<WS2812B, 6, GRB>(&(leds[S5_OFFSET]), S5_LEN);
    FastLED.addLeds<WS2812B, 20, GRB>(&(leds[S6_OFFSET]), S6_LEN);
    // extra pins: 21, 5
    leds2 = &(leds[N_LEDS/2]);

    FastLED.setBrightness(32);
    pinMode(2, OUTPUT);
    pinMode(14, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(20, OUTPUT);
    pinMode(21, OUTPUT);
    pinMode(5, OUTPUT);
    memset(leds, 0, sizeof(leds));
    FastLED.show();

    pinMode(3, INPUT_PULLUP);
    delay(50);
    g_ext_ctrl_board_enable = digitalRead(3) == 0;


    pinMode(AUDIO_INPUT_PIN, INPUT);
    analogReadResolution(ANALOG_READ_RESOLUTION);
    analogReadAveraging(ANALOG_READ_AVERAGING);

   pinMode(18, INPUT_PULLUP);
   pinMode(19, INPUT_PULLUP);
  
  /* 
   while(1) {
       //Serial.print(digitalRead(18));
       Serial.print(analogRead(8));
       Serial.print("    ");
       //Serial.println(digitalRead(19));
       Serial.println(analogRead(9));
       delay(100);
   }*/


    for (unsigned int i = 0; i < N_LEDS; ++i) {
        leds[i] = CRGB::Black;
        if ((i == S1_OFFSET) ||
            (i == S2_OFFSET) ||
            (i == S3_OFFSET) ||
            (i == S4_OFFSET) ||
            (i == S5_OFFSET) ||
            (i == S6_OFFSET)) {
            leds[i] = CRGB::Red;
            continue;
        }

        if ((i == (S1_OFFSET + S1_LEN - 1)) ||
            (i == (S2_OFFSET + S2_LEN - 1)) ||
            (i == (S3_OFFSET + S3_LEN - 1)) ||
            (i == (S4_OFFSET + S4_LEN - 1)) ||
            (i == (S5_OFFSET + S5_LEN - 1)) ||
            (i == (S6_OFFSET + S6_LEN - 1))) {
            leds[i] = CRGB::Green;
            continue;
        }

        leds[i] = CRGB::Blue;
    }


//while(1) {
    FastLED.show();
Serial.println("x");
delay(1000);
//}
    
    set_default_patterns();


    spectrumSetup();
    samplingBegin();
}

////////////////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////////////////
void fft_loop() {
  // Calculate FFT if a full sample is available.
  if (samplingIsDone()) {
    // Run FFT on sample data.
    arm_cfft_radix4_instance_f32 fft_inst;
    arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
    arm_cfft_radix4_f32(&fft_inst, samples);
    // Calculate magnitude of complex numbers output by the FFT.
    arm_cmplx_mag_f32(samples, magnitudes, FFT_SIZE);
  
      spectrumLoop();
  
    // Restart audio sampling.
    samplingBegin();
  }
    
}

inline void loop_brightness()
{
    unsigned char brightness;

    if (g_ext_ctrl_board_enable) {
        brightness = (unsigned char) (analogRead(BRIGHTNESS_PIN) >> 2);
    } else {
        brightness = 128;
    }

    if (brightness != g_brightness) {
        FastLED.setBrightness(brightness);
        g_brightness = brightness;
    }
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
                Serial.print("Pattern: ");   
                Serial.println(g_pattern);

                Serial.print("Speed: ");
                Serial.println(g_speed);

                Serial.print("Brightness: ");
                Serial.println(g_brightness);

                Serial.print("Ctrlboard: ");
                Serial.println(g_ext_ctrl_board_enable);

                //Serial.print("Free mem: ");
                //Serial.println(freeMemory());
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
    if (g_ext_ctrl_board_enable) {
        g_speed = (analogRead(SPEED_PIN) >> 3);
    } else {
        g_speed = 128;
    }
}

inline void loop_pattern() {       
    g_patterns[g_pattern].loop();

    ++g_step;
    FastLED.show();

    if (millis() > (g_pattern_last_switch_at + (2 * g_pattern_duration))) {
       advance_pattern(true);
    }
}

void loop_buttons() {
    if (digitalRead(18) == 0) {
        delay(300);
        advance_pattern(false);
    }

    if (digitalRead(19) == 0) {
        delay(300);
        advance_pattern(true);
    }
}

void loop() {
    loop_brightness();
    fft_loop();
    return;

    loop_serial();
    loop_speed();    
    loop_buttons();
    loop_pattern();
}

void efx_blink(int h, int repeats) {
    for (int cnt = 0; cnt < repeats; ++cnt) {
        for (int v = 50; v < 255; v += 3) {
            for (unsigned int i = 0; i < N_LEDS; ++i) {
                leds[i] = CHSV(h, 255, v);
            }
            FastLED.show();
        }
        for (int v = 255; v > 50; v -= 3) {
            for (unsigned int i = 0; i < N_LEDS; ++i) {
                leds[i] = CHSV(h, 255, v);
            }
            FastLED.show();
        }
    }
}




////////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Compute the average magnitude of a target frequency window vs. all other frequencies.
void windowMean(float* magnitudes, int lowBin, int highBin, float* windowMean, float* otherMean) {
    *windowMean = 0;
    *otherMean = 0;
    // Notice the first magnitude bin is skipped because it represents the
    // average power of the signal.
    for (int i = 1; i < FFT_SIZE/2; ++i) {
      if (i >= lowBin && i <= highBin) {
        *windowMean += magnitudes[i];
      }
      else {
        *otherMean += magnitudes[i];
      }
    }
    *windowMean /= (highBin - lowBin) + 1;
    *otherMean /= (FFT_SIZE / 2 - (highBin - lowBin));
}

// Convert a frequency to the appropriate FFT bin it will fall within.
int frequencyToBin(float frequency) {
  float binFrequency = float(SAMPLE_RATE_HZ) / float(FFT_SIZE);
  return int(frequency / binFrequency);
}

// Convert from HSV values (in floating point 0 to 1.0) to RGB colors usable
// by neo pixel functions.
CRGB pixelHSVtoRGBColor(float hue, float saturation, float value) {
  // Implemented from algorithm at http://en.wikipedia.org/wiki/HSL_and_HSV#From_HSV
  float chroma = value * saturation;
  float h1 = float(hue)/60.0;
  float x = chroma*(1.0-fabs(fmod(h1, 2.0)-1.0));
  float r = 0;
  float g = 0;
  float b = 0;
  if (h1 < 1.0) {
    r = chroma;
    g = x;
  }
  else if (h1 < 2.0) {
    r = x;
    g = chroma;
  }
  else if (h1 < 3.0) {
    g = chroma;
    b = x;
  }
  else if (h1 < 4.0) {
    g = x;
    b = chroma;
  }
  else if (h1 < 5.0) {
    r = x;
    b = chroma;
  }
  else // h1 <= 6.0
  {
    r = chroma;
    b = x;
  }
  float m = value - chroma;
  r += m;
  g += m;
  b += m;
  return CRGB(int(255*r), int(255*g), int(255*b));
}


////////////////////////////////////////////////////////////////////////////////
// SPECTRUM DISPLAY FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void spectrumSetup() {
  // Set the frequency window values by evenly dividing the possible frequency
  // spectrum across the number of neo pixels.
  float windowSize = (SAMPLE_RATE_HZ / 2.0) / float(NEO_PIXEL_COUNT);
  for (int i = 0; i < NEO_PIXEL_COUNT+1; ++i) {
    frequencyWindow[i] = i*windowSize;
  }
  // Evenly spread hues across all pixels.
  for (int i = 0; i < NEO_PIXEL_COUNT; ++i) {
    hues[i] = 360.0*(float(i)/float(NEO_PIXEL_COUNT-1));
  }
}

void spectrumLoop() {
  // Update each LED based on the intensity of the audio 
  // in the associated frequency window.
  float intensity, otherMean;
  for (int i = 0; i < NEO_PIXEL_COUNT; ++i) {
    windowMean(magnitudes, 
               frequencyToBin(frequencyWindow[i]),
               frequencyToBin(frequencyWindow[i+1]),
               &intensity,
               &otherMean);
    // Convert intensity to decibels.
    intensity = 20.0*log10(intensity);
    // Scale the intensity and clamp between 0 and 1.0.
    intensity -= SPECTRUM_MIN_DB;
    intensity = intensity < 0.0 ? 0.0 : intensity;
    intensity /= (SPECTRUM_MAX_DB-SPECTRUM_MIN_DB);
    intensity = intensity > 1.0 ? 1.0 : intensity;
    leds[i] = pixelHSVtoRGBColor(hues[i], 1.0, intensity);
  }
  FastLED.show();
}


////////////////////////////////////////////////////////////////////////////////
// SAMPLING FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void samplingCallback() {
  // Read from the ADC and store the sample data
  samples[sampleCounter] = (float32_t)analogRead(AUDIO_INPUT_PIN);
  Serial.println(samples[sampleCounter]);
  // Complex FFT functions require a coefficient for the imaginary part of the input.
  // Since we only have real data, set this coefficient to zero.
  samples[sampleCounter+1] = 0.0;
  // Update sample buffer position and stop after the buffer is filled
  sampleCounter += 2;
  if (sampleCounter >= FFT_SIZE*2) {
    samplingTimer.end();
  }
}

void samplingBegin() {
  // Reset sample buffer position and start callback at necessary rate.
  sampleCounter = 0;
  samplingTimer.begin(samplingCallback, 1000000/SAMPLE_RATE_HZ);
}

boolean samplingIsDone() {
  return sampleCounter >= FFT_SIZE*2;
}
