#include "BlinkyCommon.h"

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
        memset(leds, 0, sizeof(CRGB) * N_LEDS);
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

        memset(leds, 0, sizeof(CRGB) * N_LEDS);

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


