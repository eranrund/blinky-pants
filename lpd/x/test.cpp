/* LedStripXmas: A series of fun patterns for use with LED
 * strips set up as a Christmas lighting display.  You can see an
 * earlier version of this code running in this youtube video:
 * http://www.youtube.com/watch?v=VZRN0UrQSlc
 * To use this, you will need to plug the signal wire of an
 * Addressable RGB LED strip from Pololu into pin 12.
 *
 * You can optionally connect a switch between pin 3 and ground
 * to control if the Arduino automatically cycles through the
 * different patterns.  When no switch is present or the switch
 * is open, the patterns will cycle.
 *
 * You can also optionally connect a button between pin 2 and
 * ground that displays the next pattern in the series each time
 * it is pushed.  This example requires the PololuLEDStrip
 * library to be installed.
 *
 * NOTE: Timing is determined entirely by a counter incremented
 * by the main loop, and the execution time of the main loop is
 * dominated by the time it takes to write the LED colors array
 * to the LED strips.  Changing LED_COUNT will change the
 * timing, so if you like the default timing and you have fewer
 * than 509 LEDs, you might want to add an appropriate delay to
 * the main loop.  Timing is not done with the Arduino's system
 * timer because that does not work properly when this program
 * is running (the interrupts that maintain the system time must
 * be disabled while the LED strips are being updated or else
 * they will cause glitches on the LEDs).
 */
#define NUM_LEDS 228
#define DATA_PIN 6

#define NEXT_PATTERN_BUTTON_PIN  2  // button between this pin and ground
#define AUTOCYCLE_SWITCH_PIN  3  // switch between this pin and ground

// Create a buffer for holding 509 colors.
// This takes 1527 bytes and uses up most of the 2KB RAM on an Uno,
// so we should be very sparing with additional RAM use and keep
// an eye out for possible stack overflow problems.
#define LED_COUNT NUM_LEDS

#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "pixeltypes.h"




CRGB leds[LED_COUNT];
CRGB ledsX[LED_COUNT];


// system timer, incremented by one every time through the main loop
unsigned int loopCount = 0;

unsigned int seed = 0;  // used to initialize random number generator

// enumerate the possible patterns in the order they will cycle
enum Pattern {
    SimpleHSV,
    SymSimpleHSV,
    EMS,
    Flicker,
    RandomMarch,
    Flame,
    Matrix,
  WarmWhiteShimmer ,
  RandomColorWalk,
  TraditionalColors,
  ColorExplosion,
  Gradient,
  BrightTwinkle,
  Collision,
  NUM_STATES,
  AllOff = 255
};

unsigned char pattern = AllOff;
unsigned int maxLoops;  // go to next state when loopCount >= maxLoops




// initialization stuff
// 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <stdio.h>

int sockfd = 0;
void LED_show() {
    struct sockaddr_in servaddr;
    char buf[NUM_LEDS * 3];
    int i;

    for (i = 0; i < NUM_LEDS; ++i) {
        buf[ (i*3) + 0 ] = leds[i].r;
        buf[ (i*3) + 1 ] = leds[i].g;
        buf[ (i*3) + 2 ] = leds[i].b;
    }

    if (sockfd == 0) {
        sockfd = socket(AF_INET,SOCK_DGRAM,0);
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr("192.168.0.103");
    servaddr.sin_port=htons(9022);

 i =    sendto(sockfd,buf,sizeof(buf),0,
                          (struct sockaddr *)&servaddr,sizeof(servaddr));
 //printf("%d\n", i);


}

void delay(long cycles) {
    usleep(cycles*1000);
}

void handleNextPatternButton();
void SimpleHSV_pat();
void SymSimpleHSV_pat();
void EMS_pat();
void Flicker_pat();
void RandomMartch_pat();
void Flame_pat();
void Matrix_pat();
void warmWhiteShimmer(unsigned char dimOnly);
void randomColorWalk(unsigned char initializeColors, unsigned char dimOnly);
void traditionalColors();
void colorExplosionColorAdjust(unsigned char *color, unsigned char propChance,
 unsigned char *leftColor, unsigned char *rightColor);
void colorExplosion(unsigned char noNewBursts);
void brightTwinkle(unsigned char minColor, unsigned char numColors, unsigned char noNewBursts);

void gradient();
void brightTwinkleColorAdjust(unsigned char *color);
unsigned char collision();


void setup()
{
    srand(getpid() ^ time(NULL));
    for (int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = CRGB::Red;
    }



    LED_show();

}

int rand(int min, int max) {
    if (!(max-min)) return min;
    return random() % (max - min) + min;
}

// main loop
void loop()
{
  handleNextPatternButton();
//pattern = Matrix;
//
//  pattern = 2;
//
//    if (pattern < 8) pattern = 13;
    printf("%d\n", pattern);

  if (loopCount == 0)
  {
    // whenever timer resets, clear the LED colors array (all off)
    for (int i = 0; i < LED_COUNT; i++)
    {
      leds[i] = CRGB(0, 0, 0);
    }
  }

  if (pattern == WarmWhiteShimmer || pattern == RandomColorWalk)
  {
    // for these two patterns, we want to make sure we get the same
    // random sequence six times in a row (this provides smoother
    // random fluctuations in brightness/color)
    if (loopCount % 6 == 0)
    {
      seed = rand(0, 30000);
    }
    srand(seed);
  }

  // call the appropriate pattern routine based on state; these
  // routines just set the colors in the colors array
  switch (pattern)
  {
    case SimpleHSV:
        maxLoops = 400;
        SimpleHSV_pat();
        delay(6);
        break;

    case SymSimpleHSV:        
        maxLoops = 256;
        SymSimpleHSV_pat();
        delay(6);
        break;

    case EMS:
        maxLoops = LED_COUNT * 4;
        EMS_pat();
 //       delay(6);
        break;

    case Flicker:
        maxLoops = LED_COUNT * 5;
        Flicker_pat();
        break;

    case RandomMarch:
        maxLoops = LED_COUNT * 2;
        RandomMartch_pat();
        delay(24);
        break;

    case Flame:
        maxLoops = LED_COUNT;
        Flame_pat();
        break;

    case Matrix:
        maxLoops = LED_COUNT * 3;
        Matrix_pat();
        break;

    case WarmWhiteShimmer:
      // warm white shimmer for 300 loopCounts, fading over last 70
      maxLoops = 6000;
      warmWhiteShimmer(loopCount > maxLoops - 70);
      delay(2);
      break;

    case RandomColorWalk:
      // start with alternating red and green colors that randomly walk
      // to other colors for 400 loopCounts, fading over last 80
      maxLoops = 4000;
      randomColorWalk(loopCount == 0 ? 1 : 0, loopCount > maxLoops - 80);
      delay(2);
      break;

    case TraditionalColors:
      // repeating pattern of red, green, orange, blue, magenta that
      // slowly moves for 400 loopCounts
      maxLoops = 4000;
      traditionalColors();
      delay(2);
      break;

    case ColorExplosion:
      // bursts of random color that radiate outwards from random points
      // for 630 loop counts; no burst generation for the last 70 counts
      // of every 200 count cycle or over the over final 100 counts
      // (this creates a repeating bloom/decay effect)
      maxLoops = 6300;
      colorExplosion((loopCount % 200 > 130) || (loopCount > maxLoops - 100));
      delay(5);
      break;

    case Gradient:
      // red -> white -> green -> white -> red ... gradiant that scrolls
      // across the strips for 250 counts; this pattern is overlaid with
      // waves of dimness that also scroll (at twice the speed)
      maxLoops = 250;
      gradient();
      delay(20);  // add an extra 6ms delay to slow things down
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
      delay(6);
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
      delay(6);
      break;
  }

  // update the LED strips with the colors in the colors array
  LED_show();
  loopCount++;  // increment our loop counter/timer.

  if (loopCount >= maxLoops && 1) // TODO digitalRead(AUTOCYCLE_SWITCH_PIN))
  {
    // if the time is up for the current pattern and the optional hold
    // switch is not grounding the AUTOCYCLE_SWITCH_PIN, clear the
    // loop counter and advance to the next pattern in the cycle
    loopCount = 0;  // reset timer
    pattern = ((unsigned char)(pattern+1))%NUM_STATES;  // advance to next pattern
  }
}


// This function detects if the optional next pattern button is pressed
// (connecting the pin to ground) and advances to the next pattern
// in the cycle if so.  It also debounces the button.
void handleNextPatternButton()
{
  // TODO if (digitalRead(NEXT_PATTERN_BUTTON_PIN) == 0)
#if 0
  {
    // if optional button is pressed
    while (digitalRead(NEXT_PATTERN_BUTTON_PIN) == 0)
    {
      // wait for button to be released
      while (digitalRead(NEXT_PATTERN_BUTTON_PIN) == 0);
      delay(10);  // debounce the button
    }
    loopCount = 0;  // reset timer
    pattern = ((unsigned char)(pattern+1))%NUM_STATES;  // advance to next pattern
  }
#endif
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
  unsigned char walk = rand(0, directions);  // direction of random walk
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

  for (int i = 0; i < LED_COUNT; i += 2)
  {
    // randomly walk the brightness of every even LED
    randomWalk(&leds[i].red, maxBrightness, changeAmount, dimOnly ? 1 : 2);

    // warm white: red = x, green = 0.8x, blue = 0.125x
    leds[i].green = leds[i].red*4/5;  // green = 80% of red
    leds[i].blue = leds[i].red >> 3;  // blue = red/8

    // every odd LED gets set to a quarter the brighness of the preceding even LED
    if (i + 1 < LED_COUNT)
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
  switch (LED_COUNT % 7)
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

  for (int i = start; i < LED_COUNT; i+=7)
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
      leds[i] = CRGB(rand(0, maxBrightness), rand(0, maxBrightness), rand(0, maxBrightness));
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
    if (i + 1 < LED_COUNT)
    {
      leds[i+1] = leds[i-1];
    }
    if (i + 2 < LED_COUNT)
    {
      leds[i+2] = leds[i-2];
    }
  }
}


void SimpleHSV_pat()
{
    for (int i = 0; i < LED_COUNT; ++i) {
        leds[i] = CHSV((i + loopCount) % 255, 255, 255);
    }
}

void SymSimpleHSV_pat()
{
    for (int i = 0; i < LED_COUNT/2; ++i) {
        leds[i] = CHSV((i + loopCount) % 255, 255, 255);
        leds[LED_COUNT - i - 1] = leds[i];
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

  // if LED_COUNT is not an exact multiple of our repeating pattern size,
  // it will not wrap around properly, so we pick the closest LED count
  // that is an exact multiple of the pattern period (20) and is not smaller
  // than the actual LED count.
  unsigned int extendedLEDCount = (((LED_COUNT-1)/20)+1)*20;

  for (int i = 0; i < extendedLEDCount; i++)
  {
    unsigned char brightness = (loopCount - initialDarkCycles)%brighteningCycles + 1;
    unsigned char cycle = (loopCount - initialDarkCycles)/brighteningCycles;

    // transform i into a moving idx space that translates one step per
    // brightening cycle and wraps around
    unsigned int idx = (i + cycle)%extendedLEDCount;
    if (idx < LED_COUNT)  // if our transformed index exists
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
  if (*color == 31 && rand(0, propChance+1) != 0)
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

  for (int i = 1; i < LED_COUNT - 1; i++)
  {
    // adjust the colors of second through second-to-last LEDs
    colorExplosionColorAdjust(&leds[i].red, 9, &leds[i-1].red, &leds[i+1].red);
    colorExplosionColorAdjust(&leds[i].green, 9, &leds[i-1].green, &leds[i+1].green);
    colorExplosionColorAdjust(&leds[i].blue, 9, &leds[i-1].blue, &leds[i+1].blue);
  }

  // adjust the colors of the last LED
  colorExplosionColorAdjust(&leds[LED_COUNT-1].red, 9, &leds[LED_COUNT-2].red, (unsigned char*)0);
  colorExplosionColorAdjust(&leds[LED_COUNT-1].green, 9, &leds[LED_COUNT-2].green, (unsigned char*)0);
  colorExplosionColorAdjust(&leds[LED_COUNT-1].blue, 9, &leds[LED_COUNT-2].blue, (unsigned char*)0);

  if (!noNewBursts)
  {
    // if we are generating new bursts, randomly pick one new LED
    // to light up
    for (int i = 0; i < 1; i++)
    {
      int j = rand(0, LED_COUNT);  // randomly pick an LED

      switch(rand(0, 7))  // randomly pick a color
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
  while (j < LED_COUNT)
  {
    // transition from red to green over 8 LEDs
    for (int i = 0; i < 8; i++)
    {
      if (j >= LED_COUNT){ break; }
      leds[(loopCount/2 + j + LED_COUNT)%LED_COUNT] = CRGB(160 - 20*i, 20*i, (160 - 20*i)*20*i/160);
      j++;
    }
    // transition from green to red over 8 LEDs
    for (int i = 0; i < 8; i++)
    {
      if (j >= LED_COUNT){ break; }
      leds[(loopCount/2 + j + LED_COUNT)%LED_COUNT] = CRGB(20*i, 160 - 20*i, (160 - 20*i)*20*i/160);
      j++;
    }
  }

  // modify the colors array to overlay the waves of dimness
  // (since the array indices are a function of loopCount, the waves
  // of dimness scroll over time)
  const unsigned char fullDarkLEDs = 10;  // number of LEDs to leave fully off
  const unsigned char fullBrightLEDs = 5;  // number of LEDs to leave fully bright
  const unsigned char cyclePeriod = 14 + fullDarkLEDs + fullBrightLEDs;

  // if LED_COUNT is not an exact multiple of our repeating pattern size,
  // it will not wrap around properly, so we pick the closest LED count
  // that is an exact multiple of the pattern period (cyclePeriod) and is not
  // smaller than the actual LED count.
  unsigned int extendedLEDCount = (((LED_COUNT-1)/cyclePeriod)+1)*cyclePeriod;

  j = 0;
  while (j < extendedLEDCount)
  {
    unsigned int idx;

    // progressively dim the LEDs
    for (int i = 1; i < 8; i++)
    {
      idx = (j + loopCount) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= LED_COUNT){ continue; }

      leds[idx].red >>= i;
      leds[idx].green >>= i;
      leds[idx].blue >>= i;
    }

    // turn off these LEDs
    for (int i = 0; i < fullDarkLEDs; i++)
    {
      idx = (j + loopCount) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= LED_COUNT){ continue; }

      leds[idx].red = 0;
      leds[idx].green = 0;
      leds[idx].blue = 0;
    }

    // progressively bring these LEDs back
    for (int i = 0; i < 7; i++)
    {
      idx = (j + loopCount) % extendedLEDCount;
      if (j++ >= extendedLEDCount){ return; }
      if (idx >= LED_COUNT){ continue; }

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
  for (int i = 0; i < LED_COUNT; i++)
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
      int j = rand(0, LED_COUNT);
      if (leds[j].red == 0 && leds[j].green == 0 && leds[j].blue == 0)
      {
        // if the LED we picked is not already lit, pick a random
        // color for it and seed it so that it will start getting
        // brighter in that color
        switch (rand(0, numColors) + minColor)
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
        leds[0] = CRGB(rand(0, maxBrightness), rand(0, maxBrightness), rand(0, maxBrightness));
    }

    // stream is led by two full-white LEDs
    leds[1] = leds[2] = CRGB(255, 255, 255);
    // make other side of the strip a mirror image of this side
    leds[LED_COUNT - 1] = leds[0];
    leds[LED_COUNT - 2] = leds[1];
    leds[LED_COUNT - 3] = leds[2];

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
    if (startIdx < (LED_COUNT + 1)/2)
    {
      // if streams have not crossed the half-way point, keep them growing
      for (int i = 0; i < startIdx-1; i++)
      {
        // start fading previously generated parts of the stream
        fade(&leds[i].red, 5);
        fade(&leds[i].green, 5);
        fade(&leds[i].blue, 5);
        fade(&leds[LED_COUNT - i - 1].red, 5);
        fade(&leds[LED_COUNT - i - 1].green, 5);
        fade(&leds[LED_COUNT - i - 1].blue, 5);
      }
      for (int i = startIdx; i <= stopIdx; i++)
      {
        // generate new parts of the stream
        if (i >= (LED_COUNT + 1) / 2)
        {
          // anything past the halfway point is white
          leds[i] = CRGB(255, 255, 255);
        }
        else
        {
          leds[i] = leds[i-1];
        }
        // make other side of the strip a mirror image of this side
        leds[LED_COUNT - i - 1] = leds[i];
      }
      // stream is led by two full-white LEDs
      leds[stopIdx + 1] = leds[stopIdx + 2] = CRGB(255, 255, 255);
      // make other side of the strip a mirror image of this side
      leds[LED_COUNT - stopIdx - 2] = leds[stopIdx + 1];
      leds[LED_COUNT - stopIdx - 3] = leds[stopIdx + 2];
    }
    else
    {
      // streams have crossed the half-way point of the strip;
      // flash the entire strip full-brightness white (ignores maxBrightness limits)
      for (int i = 0; i < LED_COUNT; i++)
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
    for (int i = 0; i < LED_COUNT; i++)
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

#define TOP_INDEX (LED_COUNT/2)
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {iN = ( i + TOP_INDEX ) % LED_COUNT; }
  return iN;
}

int thishue = 0;
void EMS_pat() {                  //-m8-EMERGENCY LIGHTS (TWO COLOR SOLID)
  int loopCountR = (loopCount % LED_COUNT);
  int loopCountB = antipodal_index(loopCountR);
  int thathue = (thishue + 160) % 255;
  leds[loopCountR] = CHSV(thishue, 255, 255);
  leds[loopCountB] = CHSV(thathue, 255, 255);  

  if ((loopCount % 3) == 0) ++thishue;

  delay(30);
}

void Flicker_pat() {
  int random_bright = rand( 0,255);
  int random_delay = rand( 10,100);
  int random_bool = rand( 0,random_bright);
  if (random_bool < 10) {
    for(int i = 0 ; i < LED_COUNT; i++ ) {
      leds[i] = CHSV(160, 50, random_bright);
    }
    delay(random_delay);
  }

}

int adjacent_ccw(int i) {
  int r;
  if (i > 0) {r = i - 1;}
  else {r = LED_COUNT - 1;}
  return r;
}


#define copy_led_array() memcpy(&ledsX, &leds, sizeof(leds))

void RandomMartch_pat()
{
  copy_led_array();
  int iCCW;
  leds[0] = CHSV(rand( 0,255), 255, 255);
  for(int idex = 1; idex < LED_COUNT ; idex++ ) {
    iCCW = adjacent_ccw(idex);
    leds[idex].r = ledsX[iCCW][0];
    leds[idex].g = ledsX[iCCW][1];
    leds[idex].b = ledsX[iCCW][2];
  }
  if (loopCount > LED_COUNT) {
    for (int i = 0; i < loopCount - LED_COUNT; ++i) {
        if (i >= LED_COUNT) i = 0; // safety
        leds[i] = CRGB(0,0,0);
    }
  }

  LED_show();  
}

#define EVENODD (LED_COUNT % 2)
int horizontal_index(int i) {
  //-ONLY WORKS WITH INDEX < TOPINDEX
  if (i == 0) {return 0;}
  if (i == TOP_INDEX && EVENODD == 1) {return TOP_INDEX + 1;}
  if (i == TOP_INDEX && EVENODD == 0) {return TOP_INDEX;}
  return LED_COUNT - i;  
}

void Flame_pat() {
  int ghue = (0 + 80) % 255;
  int bhue = (0 + 160) % 255;
  int N3  = int(LED_COUNT/3);
  int N6  = int(LED_COUNT/6);  
  int N12 = int(LED_COUNT/12);  
  int idex = loopCount;
  int thissat = 200;
  for(int i = 0; i < N3; i++ ) {
    int j0 = (idex + i + LED_COUNT - N12) % LED_COUNT;
    int j1 = (j0+N3) % LED_COUNT;
    int j2 = (j1+N3) % LED_COUNT;    
    leds[j0] = CHSV(0 , thissat, 255);
    leds[j1] = CHSV(ghue, thissat, 255);
    leds[j2] = CHSV(bhue, thissat, 255);    
  }
  delay(10);
}

void Matrix_pat() {
    int thishue = 95;
    int thissat = 255;

  int randz = rand(0, 100);
  if (randz > 90) {
    leds[0] = CHSV(thishue, thissat, 255);
  }
  else {leds[0] = CHSV(thishue, thissat, 0);}
  copy_led_array();
    for(int i = 1; i < LED_COUNT; i++ ) {
    leds[i].r = ledsX[i-1][0];
    leds[i].g = ledsX[i-1][1];
    leds[i].b = ledsX[i-1][2];    
  }
  delay(50);
}


int main() {
    setup();
    while (true) {
        loop();
    }
}
