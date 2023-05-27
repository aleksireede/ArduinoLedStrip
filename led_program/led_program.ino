#include <FastLED.h>
#include <OneButton.h>
#include <EEPROM.h>
#include "LED_segment.h"
// Include libraries

uint8_t special_counter = 0;
uint8_t special_segment = 0;
uint8_t registered_animation = 0;

// Led Strip
#define amount_of_leds 88
struct CRGB leds[amount_of_leds];
#define pin_led_strip 3
#define strip_type WS2812B
#define color_order GRB
uint8_t brightness = 255;
uint8_t max_brightness = 255;
uint8_t min_brightness = 100;
// End Led Strip

// Button
#define green_button_pin A0  // Green Button
#define red_button_pin A5    // Red Button
#define black_button_pin 12
uint8_t button_delay = 0;
long previous_time = 0;     // used for counting seconds
long off_interval = 15000;  // 15 second delay when pressing the off button before the off text fades
// End Button

// MISC
bool save_on_shutdown = false;       // save values to rom on "shutdown" when leds go black
uint8_t base_index = 0;              // Rotating index value used by many animations
uint8_t base_speed = 0;              // base speed of all animations
uint8_t animation_mem_address = 0;   // Location we want the data to be put.
uint8_t brightness_mem_address = 4;  // Location of brightnesses value on eeprom
TBlendType currentBlending;          // NOBLEND or LINEARBLEND
bool is_running = true;              // Used to determine if we are in on or off state
bool show_brightness = false;        // this is needed so that when we want to see the brightness we don't show the animation number
bool animation_reverse = false;      // We need this when we want to chnage the direction of the animation
bool warning1 = false;               // Warning if direction is unchangeable. decimal point of first digit is lit up
// End MISC

// Sine V1 variables
uint8_t maxChanges = 24;   // Value for blending between palettes.
uint8_t thisrot = 1;       // You can change how quickly the hue rotates for this Speedy_Wave.
int8_t thisspeed = 8;      // You can change the speed of the Speedy_Wave, and use negative values.
uint8_t allfreq = 32;      // You can change the frequency, thus distance between bars.
int thisphase = 0;         // Phase change value gets calculated.
uint8_t thiscutoff = 128;  // You can change the cutoff value to display this Speedy_Wave. Lower value = longer Speedy_Wave.
uint8_t thisdelay = 30;    // You can change the delay. Also you can change the allspeed variable above.
uint8_t bgclr = 0;         // A rotating background colour.
uint8_t bgbright = 0;      // brightness of background colour
uint8_t bgclrinc = 0;
int startIndex = 0;
// End Sine V1 variables

// Sine V2 variables
CRGBPalette16 thisPalette;
CRGBPalette16 thatPalette;
uint8_t thishue;     // You can change the starting hue value for the first Speedy_Wave.
uint8_t thathue;     // You can change the starting hue for other Speedy_Wave.
uint8_t thatrot;     // You can change how quickly the hue rotates for the other Speedy_Wave. Currently 0.
uint8_t allsat;      // I like 'em fully saturated with colour.
uint8_t thisdir;     // i don't know what it does so don't touch it
uint8_t thatdir;     // i don't know what it does so don't touch it
uint8_t alldir;      // You can change direction.
int8_t thatspeed;    // You can change the speed.
int thatphase;       // Phase change value gets calculated.
uint8_t thatcutoff;  // You can change the cutoff value to display that Speedy_Wave. Lower value = longer Speedy_Wave.
uint8_t fadeval;     // Use to fade the led's of course.
// End Sine V2 variables

// Palette definitions
CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
// Use qsuba for smooth pixel colouring and qsubd for non-smooth pixel colouring
#define qsubd(x, b) ((x > b) ? b : 0)      // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b) ((x > b) ? x - b : 0)  // Analog Unsigned subtraction macro. if result <0, then => 0
uint16_t Xorig = 0x012;
uint16_t Yorig = 0x015;
uint16_t X;
uint16_t Y;
uint16_t Xn;
uint16_t Yn;
uint8_t index;
// End Palette definitions

// Buttons
OneButton S1 = OneButton(
  green_button_pin,  // Input pin for the button
  true,              // Button is active LOW
  true               // Enable internal pull-up resistor
);
OneButton S2 = OneButton(
  red_button_pin,  // Input pin for the button
  true,            // Button is active LOW
  true             // Enable internal pull-up resistor
);
OneButton S3 = OneButton(
  black_button_pin,  // Input pin for the button
  true,              // Button is active LOW
  true               // Enable internal pull-up resistor
);
// End Buttons

void setup() {
  delay(1000);
  // S1
  S1.setDebounceTicks(10);  // Prevent accidental double press
  S1.setPressTicks(500);    // that is the time when LongPressStart is called
  S1.setClickTicks(250);    // delay differentiating single clocks from double clicks

  S1.attachClick(S1_Press);                    // attach normal press to S1 (red button)
  S1.attachDuringLongPress(S1_long_press);     // attach long press to S1 (red button)
  S1.attachLongPressStop(S1_long_press_stop);  // called when long press stopped
  S1.attachDoubleClick(Check_brightness);
  // S2
  S2.setPressTicks(500);    // that is the time when LongPressStart is called
  S2.setClickTicks(250);    // delay differentiating single clocks from double clicks
  S2.setDebounceTicks(10);  // Prevent accidental double press

  S2.attachClick(S2_Press);                    // attach normal press to S2 (green button)
  S2.attachDuringLongPress(S2_long_press);     // attach long press to S2 (green button)
  S2.attachDoubleClick(Check_brightness);      // attach double press to S2 (green button)
  S2.attachLongPressStop(S2_long_press_stop);  // called when long press stops
  // S3
  S3.setDebounceTicks(10);                    // Prevent accidental double press
  S3.setClickTicks(250);                      // delay differentiating single clocks from double clicks
  S3.attachClick(ON_OFF_Press);               // attach normal press to ON_OFF_BUTTON
  S3.attachDoubleClick(WRITE_TO_EEPROM);      // write current animation to memory
  S3.attachLongPressStart(Change_Direction);  // change direction of the led strip

  pinMode(SEG_A, OUTPUT);
  pinMode(SEG_B, OUTPUT);
  pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT);
  pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);
  pinMode(SEG_DP, OUTPUT);
  pinMode(DIG_1, OUTPUT);
  pinMode(DIG_2, OUTPUT);
  pinMode(DIG_3, OUTPUT);
  pinMode(DIG_4, OUTPUT);

  FastLED.setBrightness(brightness);
  FastLED.addLeds<strip_type, pin_led_strip, color_order>(leds, amount_of_leds).setCorrection(TypicalLEDStrip);  // GRB ordering is typical
  thisPalette = RainbowColors_p;
  thatPalette = OceanColors_p;
  currentBlending = LINEARBLEND;
  currentPalette = RainbowStripeColors_p;
  resetvars();
  X = Xorig;
  Y = Yorig;  // Initialize the variables

  Startup_Animation();  // Run a single short one-time animation when powering on the led strip
  EEPROM_read();        // Read last saved vlues and return to the last saved animation and brightness
}
// End Setup

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { color_cycle, animation_custom1, animation_rainbowstripe, animation_nightsky, animation_randomcolors, animation_running_dots, animation_skycolors, custom_rainbow_animation, animation_partycolors, animation_red_purple, animation_green_blue, animation_purple_green, animation_pink_aqua, white };
uint8_t current_animation = 0;  // Index number of which pattern is current

void loop() {
  if (!is_running)  // this code will run when we press the black button
  {
    S3.tick();
    FastLED.setBrightness(0);
    FastLED.show();
    SEG_SHOW_OFF();
    return;  // exit and run loop() again
  }
  gPatterns[current_animation]();  // this calls the current animation function
  previous_time = millis();        // this keeps track of time since the device was powered on
  S1.tick();                       // check status of button 1
  S2.tick();                       // Check status of button 2
  S3.tick();                       // Check status of button 3
  FastLED.show();
  FastLED.setBrightness(brightness);
  if (show_brightness)  // this is used when we want to show the brigthness value in our 7-segment 4-digit display
  {
    uint8_t brightness_number = brightness - min_brightness;  // we want the brigthness to start from 0 so we need to subtract minimun brightness from it
    Led_segment_show(brightness_number);
    return;  // exit and run loop() again
  }
  Led_segment_show(current_animation);
  if (warning1) {
    display_segment(1, 254);
  }
}
// End Loop

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void S1_Press() {
  EVERY_N_MILLISECONDS(button_delay) {
    previous_animation();
  }
  warning1 = false;
}

void S2_long_press() {
  show_brightness = true;
  EVERY_N_MILLISECONDS(50) {
    higher_brightness();
  }
}

void Change_Direction() {
  animation_reverse = !animation_reverse;
}

void S1_long_press() {
  show_brightness = true;
  EVERY_N_MILLISECONDS(50) {
    lower_brightness();
  }
}
void Check_brightness() {
  show_brightness = !show_brightness;
}

void S2_Press() {
  EVERY_N_MILLISECONDS(button_delay) {
    next_animation();
  }
  warning1 = false;
}

void WRITE_TO_EEPROM() {
  EVERY_N_MILLISECONDS(button_delay) {
    EEPROM_write();
  }
}

void ON_OFF_Press() {
  is_running = !is_running;
}
// End Button check

void S2_long_press_stop() {
  show_brightness = false;
}

void S1_long_press_stop() {
  show_brightness = false;
}

void previous_animation() {
  if (current_animation == 0) {
    current_animation = (sizeof(gPatterns) / 2) - 1;
    return;
  }
  current_animation = (current_animation - 1) % ARRAY_SIZE(gPatterns);
}  // End Prevcious animation

void next_animation() {
  current_animation = (current_animation + 1) % ARRAY_SIZE(gPatterns);
}
// End Next Animation

void SetupRandomColorPalette() {

  targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));

}  // SetupRandomColorPalette()

void SetupSimilarColorPalette() {

  static uint8_t baseC = random8();  // You can use this as a baseline colour if you want similar hues in the next line, i.e. baseC+random8(0,5) for the hue.
  targetPalette = CRGBPalette16(CHSV(baseC + random8(0, 10), 255, random8(128, 255)), CHSV(baseC + random8(0, 10), 255, random8(128, 255)), CHSV(baseC + random8(0, 10), 192, random8(128, 255)), CHSV(baseC + random8(0, 10), 255, random8(128, 255)));

}  // SetupSimilarColorPalette()

// RainbowColors_p, RainbowStripeColors_p, OceanColors_p, CloudColors_p, ForestColors_p, and PartyColors_p.
void ChangeMe() {
  uint8_t secondHand = (millis() / 1000) % 60;  // Increase this if you want a longer demo.
  static uint8_t lastSecond = 99;               // Static variable, means it's only defined once. This is our 'debounce' variable.
  if (lastSecond == secondHand) {
    return;
  }
  lastSecond = secondHand;
  switch (secondHand) {
    case 0:
      thisrot = 1;
      thatrot = 1;
      thisPalette = PartyColors_p;
      thatPalette = PartyColors_p;
      break;
    case 5:
      thisrot = 0;
      thatdir = 1;
      thatspeed = -4;
      thisPalette = ForestColors_p;
      thatPalette = OceanColors_p;
      break;
    case 10:
      thatrot = 0;
      thisPalette = PartyColors_p;
      thatPalette = RainbowColors_p;
      break;
    case 15:
      allfreq = 16;
      thisdir = 1;
      thathue = 128;
      break;
    case 20:
      thiscutoff = 96;
      thatcutoff = 240;
      break;
    case 25:
      thiscutoff = 96;
      thatdir = 0;
      thatcutoff = 96;
      thisrot = 1;
      break;
    case 30:
      thisspeed = -4;
      thisdir = 0;
      thatspeed = -4;
      break;
    case 35:
      thiscutoff = 128;
      thatcutoff = 128;
      break;
    case 40:
      thisspeed = 3;
      break;
    case 45:
      thisspeed = 3;
      thatspeed = -3;
      break;
    case 50:
      thisspeed = 2;
      thatcutoff = 96;
      thiscutoff = 224;
      thatspeed = 3;
      break;
    case 55:
      resetvars();
      break;
    case 60:
      break;
  }
}  // ChangeMe()

void two_sin() {
  if (animation_reverse) {
    thisdir ? thisphase += beatsin8(thisspeed, 2, 10) : thisphase += beatsin8(thisspeed, 2, 10);
    thatdir ? thatphase += beatsin8(thisspeed, 2, 10) : thatphase += beatsin8(thatspeed, 2, 10);
  } else {
    thisdir ? thisphase += beatsin8(thisspeed, 2, 10) : thisphase -= beatsin8(thisspeed, 2, 10);
    thatdir ? thatphase += beatsin8(thisspeed, 2, 10) : thatphase -= beatsin8(thatspeed, 2, 10);
  }
  thishue += thisrot;  // Hue rotation is fun for thisSpeedy_Wave.
  thathue += thatrot;  // It's also fun for thatSpeedy_Wave.
  for (int k = 0; k < amount_of_leds - 1; k++) {
    int thisbright = qsuba(cubicwave8((k * allfreq) + thisphase), thiscutoff);        // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    int thatbright = qsuba(cubicwave8((k * allfreq) + 128 + thatphase), thatcutoff);  // This Speedy_Wave is 180 degrees out of phase (with the value of 128).

    leds[k] = ColorFromPalette(thisPalette, thishue, thisbright, currentBlending);
    leds[k] += ColorFromPalette(thatPalette, thathue, thatbright, currentBlending);
  }
  nscale8(leds, amount_of_leds, fadeval);
}  // two_sin()

void animation_custom1() {
  ChangeMe();
  EVERY_N_MILLISECONDS(thisdelay) {
    two_sin();  // Routine is still delay based, but at least it's now a non-blocking day.
  }
}
// End Double Sine_Wave_V1

void resetvars() {   // Reset the variable back to the beginning.
  thishue = 0;       // You can change the starting hue value for the first Speedy_Wave.
  thathue = 140;     // You can change the starting hue for other Speedy_Wave.
  thisrot = 1;       // You can change how quickly the hue rotates for this Speedy_Wave. Currently 0.
  thatrot = 1;       // You can change how quickly the hue rotates for the other Speedy_Wave. Currently 0.
  allsat = 255;      // I like 'em fully saturated with colour.
  thisdir = 0;       // Change the direction of the first Speedy_Wave.
  thatdir = 0;       // Change the direction of the other Speedy_Wave.
  alldir = 0;        // You can change direction.
  thisspeed = 4;     // You can change the speed, and use negative values.
  thatspeed = 4;     // You can change the speed, and use negative values.
  allfreq = 32;      // You can change the frequency, thus overall width of bars.
  thisphase = 0;     // Phase change value gets calculated.
  thatphase = 0;     // Phase change value gets calculated.
  thiscutoff = 192;  // You can change the cutoff value to display this Speedy_Wave. Lower value = longer Speedy_Wave.
  thatcutoff = 192;  // You can change the cutoff value to display that Speedy_Wave. Lower value = longer Speedy_Wave.
  thisdelay = 10;    // You can change the delay. Also you can change the allspeed variable above.
  fadeval = 192;     // How quickly we fade.
}  // resetvars()

void animation_nightsky_Animation() {
  warning1 = true;
  EVERY_N_SECONDS(5) {
    uint8_t baseC = random8();
    targetPalette = CRGBPalette16(CHSV(baseC - 3, 255, random8(192, 255)), CHSV(baseC + 2, 255, random8(192, 255)), CHSV(baseC + 5, 192, random8(192, 255)), CHSV(random8(), 255, random8(192, 255)));
    X = Xorig;
    Y = Yorig;
  }

  //  Xn = X-(Y/2); Yn = Y+(Xn/2);
  //  Xn = X-Y/2;   Yn = Y+Xn/2;
  //  Xn = X-(Y/2); Yn = Y+(X/2.1);
  Xn = X - (Y / 3);
  Yn = Y + (X / 1.5);
  //  Xn = X-(2*Y); Yn = Y+(X/1.1);

  X = Xn;
  Y = Yn;

  index = (sin8(X) + cos8(Y)) / 2;  // Guarantees maximum value of 255

  CRGB newcolor = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);

  //  nblend(leds[X%amount_of_leds-1], newcolor, 224);          // Try and smooth it out a bit. Higher # means less smoothing.
  nblend(leds[map(X, 0, 65535, 0, amount_of_leds)], newcolor, 224);  // Try and smooth it out a bit. Higher # means less smoothing.

  fadeToBlackBy(leds, amount_of_leds, 16);  // 8 bit, 1 = slow, 255 = fast

}  // animation_nightsky_Animation()

void FillLEDsFromPaletteColors(uint8_t colorIndex) {

  for (int i = 0; i < amount_of_leds; i++) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex + sin8(i * 16), 255);
    colorIndex += 3;
  }

}  // FillLEDsFromPaletteColors()

void ChangePalettePeriodically() {

  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if (lastSecond == secondHand) {
    return;
  }
  lastSecond = secondHand;
  CRGB p = CHSV(HUE_PURPLE, 255, 255);
  CRGB g = CHSV(HUE_GREEN, 255, 255);
  CRGB b = CRGB::Black;
  CRGB w = CRGB::White;
  switch (secondHand) {
    case 0:
      targetPalette = RainbowColors_p;
      break;
    case 10:
      targetPalette = CRGBPalette16(g, g, b, b, p, p, b, b, g, g, b, b, p, p, b, b);
      break;
    case 20:
      targetPalette = CRGBPalette16(b, b, b, w, b, b, b, w, b, b, b, w, b, b, b, w);
      break;
    case 30:
      targetPalette = LavaColors_p;
      break;
    case 40:
      targetPalette = CloudColors_p;
      break;
    case 50:
      targetPalette = PartyColors_p;
      break;
  }

}  // ChangePalettePeriodically()

void animation_randomcolors() {
  warning1 = true;
  ChangePalettePeriodically();

  EVERY_N_MILLISECONDS(100) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
  }

  EVERY_N_MILLISECONDS(thisdelay) {
    static uint8_t startIndex = 0;
    startIndex += 1;  // motion speed
    FillLEDsFromPaletteColors(startIndex);
  }
}
// End Random Crossfade palette

void animation_running_dots() {  // Eight colored dots, weaving in and out of sync with each other.
  warning1 = true;
  fadeToBlackBy(leds, amount_of_leds, 20);
  byte dothue = 0;

  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, amount_of_leds - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }

}  // animation_running-dots()

void animation_skycolors() {  // Colored stripes pulsing at a defined Beats-Per-Minute.

  uint8_t BeatsPerMinute = 62;
  currentBlending = LINEARBLEND;
  CRGBPalette16 palette = OceanColors_p;     // RainbowStripeColors_p
  uint8_t beat = beat8(BeatsPerMinute, 64);  // beatsin8(BeatsPerMinute, 64, 255) +

  for (int i = 0; i < amount_of_leds; i++) {  // 9948
    leds[i] = ColorFromPalette(palette, base_index + (i * 2), 255, currentBlending);
  }
  base_cycle(30);

}  // animation_skycolors()

void Palette_filler(uint8_t colorIndex, uint8_t speed_value) {
  // Fills the leds with palette instead of pattern function
  if (animation_reverse) {
    for (int i = amount_of_leds; i >= 0; --i) {
      leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
      colorIndex -= speed_value;
    }
  } else {
    for (int i = 0; i < amount_of_leds; ++i) {
      leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
      colorIndex += speed_value;
    }
  }

}  // Palette Filler

void animation_nightsky() {
  EVERY_N_MILLISECONDS(60) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // Blend towards the target palette
  }

  EVERY_N_MILLISECONDS(50) {
    animation_nightsky_Animation();
  }
}
// End star Night

void two_color_palette(CRGB color1, CRGB color2) {
  currentPalette = CRGBPalette16(
    color1, color1, color1, color1,
    color2, color2, color2, color2,
    color1, color1, color1, color1,
    color2, color2, color2, color2);
  currentBlending = LINEARBLEND;
  Palette_filler(base_index, 3);
  base_cycle(30);
}
void three_color_palette(CRGB color1, CRGB color2, CRGB color3) {
  currentPalette = CRGBPalette16(
    color1, color1, color2, color2,
    color3, color3, color1, color1,
    color2, color2, color3, color3,
    color1, color1, color2, color3);
  currentBlending = LINEARBLEND;
  Palette_filler(base_index, 3);
  base_cycle(30);
}

void animation_rainbowstripe() {
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  Palette_filler(base_index, 6);
  base_cycle(10);
}  // animation_rainbowstripe()

void animation_partycolors() {
  currentPalette = PartyColors_p;
  currentBlending = LINEARBLEND;
  Palette_filler(base_index, 5);
  base_cycle(10);
}

void animation_purple_green() {  // navy blue and green apple
  CRGB purple = CRGB(255, 0, 255);
  CRGB green_apple = CRGB(0, 255, 0);
  two_color_palette(purple, green_apple);
}

void animation_green_blue() {  // navy blue and green apple
  CRGB navy_blue = CRGB::Navy;
  CRGB green_apple = CRGB(0, 255, 0);
  two_color_palette(navy_blue, green_apple);
}  // Purple and Green palette

void animation_pink_aqua() {  // navy blue and green apple
  CRGB pink = CHSV(HUE_PINK, 255, 255);
  CRGB blue = CHSV(HUE_AQUA, 255, 255);
  two_color_palette(pink, blue);
}  // Purple and Green palette

void animation_red_purple() {  // purple, blue and red colors
  CRGB blue = CHSV(HUE_BLUE, 255, 255);
  CRGB purple = CHSV(HUE_PURPLE, 255, 255);
  CRGB red = CHSV(HUE_RED, 255, 255);
  three_color_palette(blue, purple, red);
}  // Purple, Blue and Red palette

void color_cycle() {
  fill_rainbow(leds, amount_of_leds, base_index, 0);
  base_cycle(25);
}  // solid rainbow

void white() {
  fill_solid(leds, amount_of_leds, CRGB(255, 255, 255));
}

void custom_rainbow_animation() {  // Rainbowish animation with changing pattern
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88(87, 220, 250);
  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = base_index * 256;  // sHue16;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis;
  sLastMillis = ms;
  if (animation_reverse) {
    sPseudotime -= deltams * msmultiplier;
  } else {
    sPseudotime += deltams * msmultiplier;
  }
  sHue16 += deltams * beatsin88(400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for (uint16_t i = 0; i < amount_of_leds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16 += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV(hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (amount_of_leds - 1) - pixelnumber;

    nblend(leds[pixelnumber], newcolor, 64);
  }
  base_cycle(25);
}  // Rainbow-ish

void Flash_Yellow() {
  FastLED.setBrightness(25);
  solid_single_color_with_short_delay(CRGB::Black);
  solid_single_color_with_short_delay(CRGB::Yellow);
  solid_single_color_with_short_delay(CRGB::Black);
  solid_single_color_with_short_delay(CRGB::Yellow);
  solid_single_color_with_short_delay(CRGB::Black);
  FastLED.setBrightness(brightness);
}

void Startup_Animation() {
  for (int i = 0; i < 200; i++) {
    repeat_one_number(i / 20);
  }
  long time_was = millis();
  while (millis() - time_was < 2500) {
    loading_animation();
    display_four_numbers(-1, 101, 9, 10);  // displays version number
  }
}

void solid_single_color_with_short_delay(CRGB color) {
  fill_solid(leds, amount_of_leds, color);
  FastLED.show();
  delay(75);
}

void EEPROM_write() {
  EEPROM.get(animation_mem_address, registered_animation);
  if (registered_animation == current_animation) {
    error_display(1);
    return;
  }
  EEPROM.put(animation_mem_address, current_animation);
  EEPROM.put(brightness_mem_address, brightness);
  Flash_Yellow();
}

void EEPROM_read() {
  EEPROM.get(animation_mem_address, current_animation);
  EEPROM.get(brightness_mem_address, brightness);
}

void error_display(uint8_t error_code) {
  long time_was = millis();
  uint8_t error_brigthness = 250;
  bool error_direction = true;
  while (millis() - time_was < 2500) {
    uint8_t time_is = millis() - time_was;
    switch (time_is) {
      case 0:
        error_brigthness = 255;
        error_direction = true;
        break;
      case 750:
        error_brigthness = 0;
        error_direction = false;
        break;
      case 1500:
        error_brigthness = 255;
        error_direction = true;
        break;
      case 2250:
        error_brigthness = 0;
        error_direction = false;
        break;
    }
    EVERY_N_MILLISECONDS(3) {
      if (error_direction) {
        error_brigthness -= 1;
      } else {
        error_brigthness += 1;
      }
    }
    fill_solid(leds, amount_of_leds, CRGB(255, 0, 0));
    FastLED.setBrightness(error_brigthness);
    FastLED.show();
    display_error(error_code);  // display error -1 -2 and so on.
  }
  FastLED.setBrightness(brightness);
}

void base_cycle(uint8_t fastness) {
  if (animation_reverse) {
    EVERY_N_MILLISECONDS(fastness) {  // Speed that effects almost all animations
      base_index--;
    }
    return;
  }
  EVERY_N_MILLISECONDS(fastness) {  // Speed that effects almost all animations
    base_index++;
  }
}

void lower_brightness() {
  if (brightness > min_brightness) {
    brightness -= 1;
    return;
  }
}

void higher_brightness() {
  if (brightness < max_brightness) {
    brightness += 1;
    return;
  }
}

void SEG_SHOW_OFF() {
  if (millis() - previous_time > off_interval) {
    repeat_one_number(-1);
    return;
  }
  display_four_numbers(-1, 0, 15, 15);  // write OFF to display. -1 is none
}

void loading_animation() {  // spins a circle indicating a loading animation on the most lef digit
  EVERY_N_MILLISECONDS(100) {
    if (special_counter > 3) {
      special_counter = 0;
    } else {
      special_counter += 1;
    }
  }
  switch (special_counter) {
    case 0:
      special_segment = 250;
      break;
    case 1:
      special_segment = 251;
      break;
    case 2:
      special_segment = 252;
      break;
    case 3:
      special_segment = 253;
      break;
  }
  display_segment(4, special_segment);
}
