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
extern int N_LEDS;

struct PatternInstance {
    void (*loop)();
    unsigned long duration;
};



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


#endif
