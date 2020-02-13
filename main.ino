
#include <FastLED.h>
#include "colorutils.h"
#include "colorpalettes.h"


#include <SerialCommand.h>





#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//This is where we adjust things to match our unique project:
#define NUM_LEDS    80      // adjust this to the number of LEDs you have: 16 or more
#define LED_TYPE    WS2812B // adjust this to the type of LEDS. This is for Neopixels
#define DATA_PIN    11      // adjust this to the pin you've connected your LEDs to   
// #define BRIGHTNESS  100      // 255 is full brightness, 128 is half, 32 is an eighth.
#define BUTTON_PIN  3       // Conanect the button to GND and one of the pins. 

#define COLOR_ORDER GBR     // Try mixing up the letters (RGB, GBR, BRG, etc) for a whole new world of color combinations

#define NUM_MODES 15         //Update this number to the highest number of "cases"

uint8_t BRIGHTNESS = 100;

uint8_t gHue = 0;           // rotating "base color" used by many of the patterns
uint16_t SPEED = 50;// SPEED set dynamically once we've started up

CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

int ledMode = 0;


byte prevKeyState = HIGH;         // button is active low

//параметры для rainbow
uint8_t thisdelay = 40;                                       // A delay value for the sequence(s)
uint8_t thishue = gHue;                                          // Starting hue value.
int8_t thisrot = 1;                                           // Hue rotation speed. Includes direction.
uint8_t deltahue = 1;                                         // Hue change between pixels.
bool thisdir = 0;

//параметры для three_sin_demo
//uint8_t thisdelay = 50;    уже есть                                    // A delay value for the sequence(s)

int wave1=0;                                                  // Current phase is calculated.
int wave2=0;
int wave3=0;

uint8_t inc1 = 2;                                             // Phase shift. Keep it low, as this is the speed at which the waves move.
uint8_t inc2 = 1;
uint8_t inc3 = -2;

uint8_t lvl1 = 80;                                            // Any result below this value is displayed as 0.
uint8_t lvl2 = 80;
uint8_t lvl3 = 80;

uint8_t mul1 = 20;                                            // Frequency, thus the distance between waves
uint8_t mul2 = 25;
uint8_t mul3 = 22;


SerialCommand sCmd;

//------------------SETUP------------------
void setup() {
  Serial.begin(9600);
  delay( 2000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  currentBlending;

  // pinMode(BUTTON_PIN, INPUT_PULLUP);
  sCmd.addCommand("P",  change_pattern);//pattern
  sCmd.addCommand("B",  set_brightness);//BRIGHTNESS
  sCmd.addCommand("S",  set_speed);//SPEEd
  sCmd.setDefaultHandler(unrecognized);
  Serial.println("SPACE_STICK");
}

void unrecognized(const char *command) {
  Serial.println("What?");
}
//------------------MAIN LOOP------------------
void loop() {
  sCmd.readSerial();

  switch (ledMode) {
    case 0:  rainbow(); break;

    case 1:  juggle();  break;

    case 2:  sinelon(); break;

    case 3:  confetti(); break;

    case 4:  rainbowWithGlitter();  break;

    case  5: thisrot = 1; deltahue = 5; rainbow(); break;

    case  6: thisdir = -1; deltahue = 10; rainbow();break;

    case 7: thisrot = 5; rainbow(); break;

    case 8: thisrot = 5; thisdir = -1; deltahue = 20; rainbow(); break;

    case 9: deltahue = 30 ;rainbow(); break;

    case 10: deltahue = 2; thisrot = 5; rainbow(); break;

    case 11: three_sin(); break;

    case 12: thisdelay = 25; mul1 = 20; mul2 = 25; mul3 = 22; lvl1 = 80; lvl2 = 80; lvl3 = 80; inc1 = 1; inc2 = 1;three_sin(); break;

    case 13: mul1 = 5; mul2 = 8; mul3 = 7; three_sin(); break;

    case 14:thisdelay = 40; lvl1 = 180; lvl2 = 180; lvl3 = 180; three_sin(); break;

    case 15:blendwave(); break;
  }
  FastLED.show();
  FastLED.delay(1000 / SPEED);

  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
    
  }

}

void set_brightness(){
  char *arg;
  arg = sCmd.next(); // getting argument
   if (arg != NULL) 
  {   
      BRIGHTNESS = atoi(arg);
      FastLED.setBrightness(BRIGHTNESS);
  }
}

void set_speed(){
  char *arg;
  arg = sCmd.next(); // getting argument
   if (arg != NULL) 
  {   
      SPEED = atoi(arg);
  }
}

void change_pattern() {
  char *arg;
  arg = sCmd.next(); // getting argument

  if (arg != NULL) 
  {   
      ledMode = atoi(arg);
  }
  else
  {
    ledMode = random8(0,15); // if mode is null we get random values
  }
 
  if (ledMode > NUM_MODES && ledMode < 0) { // If arg will be not good number
    ledMode = 0;
  }
  Serial.println(ledMode);
}

//GLITTER
void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::Gray;
  }
}



// I use a direction variable instead of signed math so I can use it in multiple routines.

void three_sin() {
  wave1 += inc1;
  wave2 += inc2;
  wave3 += inc3;
  for (int k=0; k<NUM_LEDS; k++) {
    leds[k].r = qsub8(sin8(mul1*k + wave1/128), lvl1);        // Another fixed frequency, variable phase sine wave with lowered level
    leds[k].g = qsub8(sin8(mul2*k + wave2/128), lvl2);        // A fixed frequency, variable phase sine wave with lowered level
    leds[k].b = qsub8(sin8(mul3*k + wave3/128), lvl3);        // A fixed frequency, variable phase sine wave with lowered level
  }
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  if (thisdir == 0) thishue+= thisrot; else thishue -= thisrot; // I could use signed math, but 'thisdir' works with other routines.
  fill_rainbow(leds, NUM_LEDS, thishue, deltahue);
  // fadeToBlackBy( leds, NUM_LEDS, 255-BRIGHTNESS); 
}


CRGB clr1;
CRGB clr2;
uint8_t speed;
uint8_t loc1;
uint8_t loc2;
uint8_t ran1;
uint8_t ran2;

void blendwave() {

  speed = beatsin8(6,0,200);

  clr1 = blend(CHSV(beatsin8(3,0,200),200,200), CHSV(beatsin8(4,0,200),200,200), speed);
  clr2 = blend(CHSV(beatsin8(4,0,200),200,200), CHSV(beatsin8(3,0,200),200,200), speed);

  loc1 = beatsin8(10,0,NUM_LEDS-1);
  
  fill_gradient_RGB(leds, 0, clr2, loc1, clr1);
  fill_gradient_RGB(leds, loc1, clr2, NUM_LEDS-1, clr1);

} // blendwave()




void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}



void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 255, BRIGHTNESS * 2); //Adjusted Brightness with variable
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 16);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 255, BRIGHTNESS * 2); //Adjusted Brightness with variable
    dothue += 32;
  }
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 16);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, BRIGHTNESS * 2);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = RainbowColors_p; //can adjust the palette here
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }



}
