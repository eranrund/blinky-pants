#include "BlinkyCommon.h"

class BlinkyFaderPattern1 : public BasePattern {
public:
    int v;
    bool dir;

    int balance;
    bool balance_dir;

    BlinkyFaderPattern1() : BasePattern() {
        v = 0;
        dir = true;
        balance = 0;
        balance_dir = true;
    }

   void loop1() {
       CHSV c = CHSV(g_step >> 4, 255,
                    200 + (55 * sin(( (g_step % 200) * 2 * PI / 200))));

        for (unsigned int i = 0; i < N_LEDS; ++i) {
            leds[i] = c;
        }

        inc();
        FastLED.delay(1);
    }

    void loop2() {
        int state = (g_step >> 9) % 9;

        loop2(
            (state >> 0) & 1,
            (state >> 1) & 1,
            (state >> 2) & 1
        );
    }

     void loop2(bool compl_colors, bool cos_sin1, bool interleave) {
         loop2(compl_colors, cos_sin1, interleave, 0, N_LEDS, 0);
        inc();
        FastLED.delay(4);
    }

    

     void loop2(bool compl_colors, bool cos_sin1, bool interleave, int start, unsigned int n_leds, int h_offset) {
         CHSV c1, c2;
        
         c1 = CHSV(h_offset + (g_step >> 4) % 255, 255,
            200 + (55 * sin(( (g_step % 150) * 2 * PI / 150))));  
         c2 = CHSV(
                    h_offset + (compl_colors ? 128 : 0) + (g_step >> 4), 255,
                    200 + (-55 * (cos_sin1 ? sin : cos)( (g_step % 150) * 2 * PI / 150))
            );



        for (unsigned int i = 0; i < n_leds / 2; ++i) {
            leds[start + (interleave ? i * 2 : i)] = c1;
            leds[start + (interleave ? (i * 2) + 1 : i + (n_leds/2))] = c2;
        }

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
        FastLED.delay(20 * sin( (g_step % 150) * 2*PI / 150 ) + 60);
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
