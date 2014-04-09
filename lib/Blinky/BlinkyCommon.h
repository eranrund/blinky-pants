#ifndef __COMMONFRAMEWORK_H__
#define __COMMONFRAMEWORK_H__

#define arr_len(a) (sizeof(a) / sizeof((a)[0]))

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


////////////////////////////////////////////////////////////////////////////////
// TODO
////////////////////////////////////////////////////////////////////////////////
#include "FastSPI_LED2.h"
extern CRGB leds[];
extern CRGB * leds2;
extern CRGB ledsX[];
extern int N_LEDS;
extern unsigned char g_speed;
extern unsigned char g_brightness;
extern unsigned int g_step;
extern unsigned int seed;

#define copy_led_array() memcpy(ledsX, leds, sizeof(CRGB)*N_LEDS)
#define uncopy_led_array() memcpy(leds, ledsX, sizeof(CRGB)*N_LEDS)
#define speed_delay(min, max) delay(map(g_speed, 0, 0xff, min, max));


struct PatternInstance {
    void (*loop)();
    unsigned long duration;
};

template <bool b> struct StaticAssert {};
template <> struct StaticAssert<true> { static void assert() {} };


////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

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




////////////////////////////////////////////////////////////////////////////////
// Step Generator
////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////
// Base Pattern
////////////////////////////////////////////////////////////////////////////////
class LedRange {
public:
    unsigned char start;
    unsigned char end;
};

class BasePattern {
public:
    BasePattern() {
    }

    void loop() {
    }

};

class BasePattern2 : public BasePattern {
public:
    const LedRange * ranges;
    unsigned char n_ranges;
    unsigned short n_leds;

    BasePattern2(const LedRange * ranges, unsigned char n_ranges) : BasePattern() {
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


#endif
