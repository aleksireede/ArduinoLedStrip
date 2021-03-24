#include <FastLED.h>
#define NUM_LEDS 88
CRGB leds[NUM_LEDS];
#define ON_OFF_PIN 4
#define BW_PIN 5
#define FW_PIN 6
#define LED_STRIP_PIN 3
bool Prev_SW_state = false;
bool ON = true;
bool Prev_on_off_state = false;
bool prevPattern_state = false;
bool nextPattern_state = false;
uint8_t BRIGHTNESS = 255;
int SPEED = 120;

CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup() {
  Serial.begin(9600);
  pinMode(ON_OFF_PIN, INPUT_PULLUP);
  pinMode(BW_PIN, INPUT_PULLUP);
  pinMode(FW_PIN, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, LED_STRIP_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {
  solid_white,
  solid_red,
  solid_green,
  solid_blue,
  rainbow_ish,
  confetti,
  sinelon,
  juggle,
  Palette_RWB,
  Palette_PG,
  rainbow,
  solid_rainbow
};//Pattern List

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // Rotating "base color" used by many of the patterns
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void loop() {

  if (ON) {//Checks if the leds are ON or OFF
    gPatterns[gCurrentPatternNumber]();
  }
  else {
    fadeToBlackBy( leds, NUM_LEDS, 20);
  }
  Serial.println(gCurrentPatternNumber);
  FastLED.setBrightness(  BRIGHTNESS );//Set led strip Brgihtness
  FastLED.show();//update led Strip With current color
  FastLED.delay(1000 / SPEED);

  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;
  }
  if ((digitalRead(FW_PIN) == LOW) & (nextPattern_state == false)) {
    nextPattern();
    nextPattern_state = true;
  }
  if (digitalRead(FW_PIN) == HIGH) {
    nextPattern_state = false;
  }
  if ((digitalRead(BW_PIN) == LOW) & (prevPattern_state == false)) {
    prevPattern();
    prevPattern_state = true;
  }
  if (digitalRead(BW_PIN) == HIGH) {
    prevPattern_state = false;
  }
  if ((digitalRead(ON_OFF_PIN) == LOW) & (Prev_on_off_state == false)) {
    ON = !ON;
    Prev_on_off_state = true;
  }
  if (digitalRead(ON_OFF_PIN) == HIGH) {
    Prev_on_off_state = false;
  }
}

void nextPattern()
{ //next pattern
  if (gCurrentPatternNumber < sizeof(gPatterns)) {
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  }
  else {
    gCurrentPatternNumber = 0;
  }
}

void prevPattern()
{ //previous pattern
  if (gCurrentPatternNumber > 0) {
    gCurrentPatternNumber = (gCurrentPatternNumber - 1) % ARRAY_SIZE( gPatterns);
  }
  else {
    gCurrentPatternNumber = ARRAY_SIZE( gPatterns) - 1;
  }
}


void rainbow_ish()
{ //Rainbowish animation with changing pattern
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS - 1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }
}


void confetti()
{
  // Random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // A colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void juggle() {
  // Eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


void FillLEDsFromPaletteColors( uint8_t colorIndex){
  //Fills the leds with palette instead of pattern function
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
  CRGB::Red,
  CRGB::Gray, // 'white' is too bright compared to red and blue
  CRGB::Blue,
  CRGB::Black,

  CRGB::Red,
  CRGB::Gray,
  CRGB::Blue,
  CRGB::Black,

  CRGB::Red,
  CRGB::Red,
  CRGB::Gray,
  CRGB::Gray,
  CRGB::Blue,
  CRGB::Blue,
  CRGB::Black,
  CRGB::Black
};

void Palette_RWB() {
  //Red white and blue colors
  currentPalette = myRedWhiteBluePalette_p;
  currentBlending = LINEARBLEND;
  FillLEDsFromPaletteColors( gHue);
}

void Palette_PG() {
  //purple and green colors
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;

  currentPalette = CRGBPalette16(
                     green,  green,  green,  black,
                     purple, purple, purple,  black,
                     green,  green,  green,  black,
                     purple, purple, purple,  black );
  currentBlending = LINEARBLEND;
  FillLEDsFromPaletteColors( gHue);
}

void solid_white() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
}

void solid_red() {
  fill_solid(leds, NUM_LEDS, CRGB::Red);
}

void solid_green() {
  fill_solid(leds, NUM_LEDS, CRGB::Green);
}

void solid_blue() {
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
}

void rainbow() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void solid_rainbow() {
  fill_solid(leds, NUM_LEDS, CHSV(gHue,255,255));
}
