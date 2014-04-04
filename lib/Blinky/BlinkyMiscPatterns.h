////////////////////////////////////////////////////////////////////////////////
// 
// A lot copied from:
// https://github.com/pololu/pololu-led-strip-arduino/blob/master/PololuLedStrip/examples/LedStripXmas/LedStripXmas.ino
////////////////////////////////////////////////////////////////////////////////

#include "BlinkyCommon.h"

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Warm White Shimmer
////////////////////////////////////////////////////////////////////////////////

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

// This function randomly increases or decreases the brightness of the
// even red LEDs by changeAmount, capped at maxBrightness.  The green
// and blue LED values are set proportional to the red value so that
// the LED color is warm white.  Each odd LED is set to a quarter the
// brightness of the preceding even LEDs.  The dimOnly argument
// disables the random increase option when it is true, causing
// all the LEDs to get dimmer by changeAmount; this can be used for a
// fade-out effect.
void warmWhiteShimmer()
{
  unsigned char dimOnly = g_step > 3000 - 70;
  const unsigned char maxBrightness = 220;  // cap on LED brighness
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
    
    if (g_step % 6 == 0) {
        seed = random(30000);
    }
    randomSeed(seed);

    speed_delay(0, 50);
}


////////////////////////////////////////////////////////////////////////////////
// HSV patterns
////////////////////////////////////////////////////////////////////////////////


void SymSimpleHSV_pat()
{
    for (int i = 0; i < N_LEDS/2; ++i) {
       leds[i] = CHSV((i + g_step) % 255, 255, 255);
       leds[N_LEDS - i - 1] = leds[i];
    }
    speed_delay(0, 50);
}


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
                    start_hsv +  (i + g_step) % num_hsvs,
                255, 255);
            setRange(i, rgb);
            setRange((n_ranges / 2) + i, rgb);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
// Traditional Xmas Colors
////////////////////////////////////////////////////////////////////////////////

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
  const unsigned char brighteningCycles = map(g_speed, 0, 0xff, 0, 55);

  // if N_LEDS is not an exact multiple of our repeating pattern size,
  // it will not wrap around properly, so we pick the closest LED count
  // that is an exact multiple of the pattern period (20) and is not smaller
  // than the actual LED count.
  unsigned int extendedLEDCount = (((N_LEDS-1)/20)+1)*20;

  for (int i = 0; i < extendedLEDCount; i++)
  {
    unsigned char brightness = (g_step - initialDarkCycles)%brighteningCycles + 1;
    unsigned char cycle = (g_step - initialDarkCycles)/brighteningCycles;

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

////////////////////////////////////////////////////////////////////////////////
// Bright Twinkle + Color Explosion
////////////////////////////////////////////////////////////////////////////////

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
void colorExplosion()
{
    unsigned char noNewBursts = (g_step % 200 > 130) || (g_step > 630 - 100);


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

  if (g_step == 629) {
      g_step = 0;
    }
  speed_delay(0, 70);
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
void brightTwinkle_helper(unsigned char minColor, unsigned char numColors, unsigned char noNewBursts)
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

void brightTwinkle()
{
      // random LEDs light up brightly and fade away; it is a very similar
      // algorithm to colorExplosion (just no radiating outward from the
      // LEDs that light up); as time goes on, allow progressively more
      // colors, halting generation of new twinkles for last 100 counts.
      unsigned short maxLoops = 1200;
      if (g_step < 400)
      {
        brightTwinkle_helper(0, 1, 0);  // only white for first 400 g_steps
      }
      else if (g_step < 650)
      {
        brightTwinkle_helper(0, 2, 0);  // white and red for next 250 counts
      }
      else if (g_step < 900)
      {
        brightTwinkle_helper(1, 2, 0);  // red, and green for next 250 counts
      }
      else
      {
        // red, green, blue, cyan, magenta, yellow for the rest of the time
        brightTwinkle_helper(1, 6, g_step > maxLoops - 100);
      }

    if (g_step >= maxLoops) {
        g_step = 0;
    }
    speed_delay(0, 100);
}



////////////////////////////////////////////////////////////////////////////////
// Gradient
////////////////////////////////////////////////////////////////////////////////

// This function creates a scrolling color gradient that smoothly
// transforms from red to white to green back to white back to red.
// This pattern is overlaid with waves of brightness and dimness that
// scroll at twice the speed of the color gradient.
void gradient()
{
  unsigned int j = 0;

  // populate colors array with full-brightness gradient colors
  // (since the array indices are a function of g_step, the gradient
  // colors scroll over time)
  while (j < N_LEDS)
  {
    // transition from red to green over 8 LEDs
    for (int i = 0; i < 8; i++)
    {
      if (j >= N_LEDS){ break; }
      leds[(g_step/2 + j + N_LEDS)%N_LEDS] = CRGB(160 - 20*i, 20*i, (160 - 20*i)*20*i/160);
      j++;
    }
    // transition from green to red over 8 LEDs
    for (int i = 0; i < 8; i++)
    {
      if (j >= N_LEDS){ break; }
      leds[(g_step/2 + j + N_LEDS)%N_LEDS] = CRGB(20*i, 160 - 20*i, (160 - 20*i)*20*i/160);
      j++;
    }
  }

  // modify the colors array to overlay the waves of dimness
  // (since the array indices are a function of g_step, the waves
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
      idx = (j + g_step) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= N_LEDS){ continue; }

      leds[idx].red >>= i;
      leds[idx].green >>= i;
      leds[idx].blue >>= i;
    }

    // turn off these LEDs
    for (int i = 0; i < fullDarkLEDs; i++)
    {
      idx = (j + g_step) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= N_LEDS){ continue; }

      leds[idx].red = 0;
      leds[idx].green = 0;
      leds[idx].blue = 0;
    }

    // progressively bring these LEDs back
    for (int i = 0; i < 7; i++)
    {
      idx = (j + g_step) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= N_LEDS){ continue; }

      leds[idx].red >>= (7 - i);
      leds[idx].green >>= (7 - i);
      leds[idx].blue >>= (7 - i);
    }

    // skip over these LEDs to leave them at full brightness
    j += fullBrightLEDs;
  }

  if (g_step == 249) {
      g_step = 0;
    }
  speed_delay(0, 100);
}


////////////////////////////////////////////////////////////////////////////////
// Collision
////////////////////////////////////////////////////////////////////////////////

// This function spawns streams of color from each end of the strip
// that collide, at which point the entire strip flashes bright white
// briefly and then fades.  Unlike the other patterns, this function
// maintains a lot of complicated state data and tells the main loop
// when it is done by returning 1 (a return value of 0 means it is
// still in progress).
unsigned char collision_helper()
{
  const unsigned char maxBrightness = 255;  // max brightness for the colors
  const unsigned char numCollisions = 5;  // # of collisions before pattern ends
  static unsigned char state = 0;  // pattern state
  static unsigned int count = 0;  // counter used by pattern

  if (g_step == 0)
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
void collision() {

    if (collision_helper()) {
        g_step = 0;
    }
    speed_delay(0, 30);
}


////////////////////////////////////////////////////////////////////////////////
// EMS
////////////////////////////////////////////////////////////////////////////////

#define TOP_INDEX (N_LEDS/2)
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {iN = ( i + TOP_INDEX ) % N_LEDS; }
  return iN;
}

//int thishue = 0;
void EMS_pat() {                  //-m8-EMERGENCY LIGHTS (TWO COLOR SOLID)
  int g_stepR = (g_step % N_LEDS);
  int g_stepB = antipodal_index(g_stepR);
  int thishue = g_step / 3;
  int thathue = (thishue + 160) % 255;
  leds[g_stepR] = CHSV(thishue, 255, 255);
  leds[g_stepB] = CHSV(thathue, 255, 255);  

  //if ((g_step % 3) == 0) ++thishue;

  speed_delay(0, 120);
}


////////////////////////////////////////////////////////////////////////////////
// Flicekr
////////////////////////////////////////////////////////////////////////////////

void Flicker_pat() {
  int random_bright = random(0, g_brightness < 100 ? 200 : 255);
  int random_delay = random(10,100);
  int random_bool = random(0,random_bright);
  if (random_bool < 10) {
      CHSV c = CHSV(160, 50, random_bright);
    for(int i = 0 ; i < N_LEDS; i++ ) {
      leds[i] = c;
    }
      delay(map(g_speed, 0, 0xff,random_delay / 4, random_delay * 4));
    
  }
}


////////////////////////////////////////////////////////////////////////////////
// Random March
////////////////////////////////////////////////////////////////////////////////

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
  if (g_step > N_LEDS) {
    for (int i = 0; i < g_step - N_LEDS; ++i) {
        if (i >= N_LEDS) break;
        leds[i] = CRGB(0,0,0);
    }
  }

  speed_delay(10, 200);
}


////////////////////////////////////////////////////////////////////////////////
// Flame
////////////////////////////////////////////////////////////////////////////////

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
  int idex = g_step;
  int thissat = 200;
  for(int i = 0; i < N3; i++ ) {
    int j0 = (idex + i + N_LEDS - N12) % N_LEDS;
    int j1 = (j0+N3) % N_LEDS;
    int j2 = (j1+N3) % N_LEDS;    
    leds[j0] = CHSV(0 , thissat, 255);
    leds[j1] = CHSV(ghue, thissat, 255);
    leds[j2] = CHSV(bhue, thissat, 255);    
  }
  speed_delay(5, 130);
}


////////////////////////////////////////////////////////////////////////////////
// Matrix
////////////////////////////////////////////////////////////////////////////////


void Matrix_pat() {
    int thishue = 95;
    int thissat = 255;

  int rand = random(0, 100);
  if ((rand > 90) || (g_step == 0)) {
    leds[0] = CHSV(thishue, thissat, 255);
  }
  else {leds[0] = CHSV(thishue, thissat, 0);}
  copy_led_array();
    for(int i = 1; i < N_LEDS; i++ ) {
    leds[i].r = ledsX[i-1][0];
    leds[i].g = ledsX[i-1][1];
    leds[i].b = ledsX[i-1][2];    
  }

    speed_delay(5, 300);
}



////////////////////////////////////////////////////////////////////////////////
// Rings
////////////////////////////////////////////////////////////////////////////////

class RingsPatterns : public BasePattern2 {
public:
    int speed_cnt;

   RingsPatterns(const LedRange * ranges, unsigned char n_ranges) : BasePattern2(ranges, n_ranges) {
    speed_cnt = 0;

    }

    void loop_shoot() {
        int half = n_ranges / 2;
        for (int i = 0; i < half; ++i) {
            int cur_range = g_step % half;
            CRGB rgb;
            if (cur_range == i) {
                rgb = CHSV(g_step % 255, 255, 255);
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
            int led_idx = map(g_step % 30, 0, 30, 0, range_leds);

            if (sym && (i >= (n_ranges/2))) {
                led_idx = range_leds - led_idx - 1;
            }

 
            for (int j = 0; j < 3; ++j) {
                leds[ranges[i].start + ((led_idx + j) % range_leds)] = CHSV((g_step + (j*2)) % 255, 255, 255);
            }
        }
    }

    void delay() {
        ::delay(max(0, map(g_speed, 0, 0xff, 0, 80) + (30 * cos((g_step % 400) * 2 * PI / 400))));
    }
};

