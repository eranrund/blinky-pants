#include "FastSPI_LED2.h"
#include "FreeIMU.h"

#define N_LEDS 24

CRGB leds[N_LEDS];
CRGB * leds2 = (leds + (N_LEDS/2));

CRGB ledsX[N_LEDS];
#define copy_led_array() memcpy(ledsX, leds, sizeof(leds))
#define uncopy_led_array() memcpy(leds, ledsX, sizeof(leds))
 

bool g_sw = false;
FreeIMU my3IMU = FreeIMU();


void setup()
{
    Serial.begin(9600);
    Serial.println("OK");

    FastLED.addLeds<WS2812B, 13, GRB>(leds, N_LEDS);
    FastLED.setBrightness(150);

    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CRGB::Green;
    }
    FastLED.show();

    Wire.begin();
    Serial.println("IMU");
    my3IMU.init(true);
    Serial.println("DONE");

    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
}

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
            leds[i] = CHSV(step >> 2, 255,
                    200 + (55 * sin(( (step % 200) * 2 * PI / 200)))
            );
        }

        inc();
        delay(1);
    }

    void loop2() {
        int state = (step >> 8) % 9;

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
        delay(2);
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

class FlickerPattern : public BasePattern {
public:
    unsigned long next_change;
    bool dir;

    FlickerPattern() : BasePattern(0xffffffff) {
        dir = false;
        next_change = 0;
    }

    void loop1() {
        unsigned long now = millis();
        if (now > next_change) {
            next_change = now + 100;
            dir = !dir;

            memset(leds, dir ? 0xff : 0, sizeof(leds));
            FastLED.show();
        }

    }
};
FlickerPattern FlickerPattern1;

void HSV_loop() {
    static unsigned char h = 0;
    for (int i = 0; i < N_LEDS; ++i) {
        leds[i] = CHSV( (h + i) % 255, 255, 255);
    }
    FastLED.show();
    ++h;
    delay(15);
}


float ypr[3];
void loop() {
    static unsigned long last_switch_at = 0;
    static unsigned char pattern = 0;
    unsigned long pattern_time = 0;

    my3IMU.getYawPitchRoll(ypr);
    //FastLED.setBrightness(map(ypr[2], -90, 90, 0, 255)); 
    //Serial.print(ypr[0]); Serial.print("     "); Serial.print(ypr[1]); Serial.print("     "); Serial.print(ypr[2]); Serial.println(); 

    if (ypr[2] < -30) {
        FlickerPattern1.loop1();
        return;
    } 

    
    switch (pattern) {
        case 0:
            FaderPattern1.loop1();
            pattern_time = 5000;
            break;

        case 1:
            FaderPattern1.loop2();
            pattern_time = 37000;
            break;

        case 2:
            FaderPattern1.loop3();
            pattern_time = 15000;
            break;

        case 3:
            CollisionPattern.loop();
            pattern_time = 8000;
            break;

        case 4:
            HSV_loop();
            pattern_time = 20000;
            break;
    }


    if (millis() > (last_switch_at + pattern_time)) {
        pattern = (pattern + 1) % 5;
        last_switch_at = millis();
    }
}
