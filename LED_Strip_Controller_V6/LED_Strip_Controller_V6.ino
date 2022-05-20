#include <FastLED.h>
#include <OneButton.h>
#include <EEPROM.h>
// Inlcude libraries

// Led Strip
#define LED_COUNT 88
struct CRGB leds[LED_COUNT];
#define LED_STRIP_PIN 3
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
uint8_t brightness = 255;
uint8_t max_brightness = 255;
uint8_t min_brightness = 100;
// End Led Strip

// Button
#define S1_pin A0 // Vihr
#define S2_pin A5 // PUN
uint8_t button_delay = 0;
#define S3_pin_no 12
long previousMillis = 0; // used for counting seconds
long off_interval = 15000; // 15 second delay when pressing the off button before the off text fades
// End Button

// 7-segment display 4-digit
#define SEG_A 4
#define SEG_B 5
#define SEG_C 6
#define SEG_D 7
#define SEG_E 8
#define SEG_F 9
#define SEG_G 10
#define SEG_DP 11
#define DIG_1 A1
#define DIG_2 A2
#define DIG_3 A3
#define DIG_4 A4
// END 7-segment display 4-digit

// MISC
bool save_on_shutdown = false;        // save values to rom on "shutdown" when leds go black
uint8_t base_index = 0;               // Rotating index value used by many animations
uint8_t base_speed = 0;               // base speed of all animations
uint8_t animation = 0;                // Initialize current animation
uint8_t animation_count = 11;         // Count animations (Used by button counter to prevent overflow)
uint8_t animation_mem_address = 0;    // Location we want the data to be put.
uint8_t brightness_mem_address = 4;   // Location of brightness on eeprom
TBlendType currentBlending;           // NOBLEND or LINEARBLEND
bool ON_OFF_STATE = true;
bool BRIGHTNESS_MODIFY = false;
bool BRIGHTNESS_MODIFY_check = false;
bool Reverse_Direction = false;
// End MISC

// Sine V1 variables
uint8_t maxChanges = 24; // Value for blending between palettes.
uint8_t thisrot = 1;      // You can change how quickly the hue rotates for this Speedy_Wave.
int8_t thisspeed = 8;     // You can change the speed of the Speedy_Wave, and use negative values.
uint8_t allfreq = 32;     // You can change the frequency, thus distance between bars.
int thisphase = 0;        // Phase change value gets calculated.
uint8_t thiscutoff = 128; // You can change the cutoff value to display this Speedy_Wave. Lower value = longer Speedy_Wave.
uint8_t thisdelay = 30;   // You can change the delay. Also you can change the allspeed variable above.
uint8_t bgclr = 0;        // A rotating background colour.
uint8_t bgbright = 0;     // brightness of background colour
uint8_t bgclrinc = 0;
int startIndex = 0;
// End Sine V1 variables

// Sine V2 variables
CRGBPalette16 thisPalette;
CRGBPalette16 thatPalette;
uint8_t thishue;            // You can change the starting hue value for the first Speedy_Wave.
uint8_t thathue;            // You can change the starting hue for other Speedy_Wave.
uint8_t thatrot;            // You can change how quickly the hue rotates for the other Speedy_Wave. Currently 0.
uint8_t allsat;             // I like 'em fully saturated with colour.
uint8_t thisdir;
uint8_t thatdir;
uint8_t alldir;     // You can change direction.
int8_t thatspeed;   // You can change the speed.
int thatphase;      // Phase change value gets calculated.
uint8_t thatcutoff; // You can change the cutoff value to display that Speedy_Wave. Lower value = longer Speedy_Wave.
uint8_t fadeval;    // Use to fade the led's of course.
// End Sine V2 variables

// Palette definitions
CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
// Use qsuba for smooth pixel colouring and qsubd for non-smooth pixel colouring
#define qsubd(x, b) ((x > b) ? b : 0)     // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b) ((x > b) ? x - b : 0) // Analog Unsigned subtraction macro. if result <0, then => 0
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
                 S1_pin,  // Input pin for the button
                 true,        // Button is active LOW
                 true         // Enable internal pull-up resistor
               );
OneButton S2 = OneButton(
                 S2_pin,  // Input pin for the button
                 true,        // Button is active LOW
                 true         // Enable internal pull-up resistor
               );
OneButton S3 = OneButton(
                 S3_pin_no,  // Input pin for the button
                 true,        // Button is active LOW
                 true         // Enable internal pull-up resistor
               );
// End Buttons

void setup()
{
  Serial.begin(9600);
  delay(1000);
  // S1
  S1.setDebounceTicks(10);// Prevent accidental double press
  S1.setPressTicks(500); // that is the time when LongPressStart is called
  S1.setClickTicks(250); // delay differentiating single clocks from double clicks

  S1.attachClick(S1_Press); // attach normal press to S1 (red button)
  S1.attachDuringLongPress(S1_long_press); // attach long press to S1 (red button)
  S1.attachLongPressStop(S1_long_press_stop); // called when long press stopped
  S1.attachDoubleClick(Check_brightness);
  // S2
  S2.setPressTicks(500); // that is the time when LongPressStart is called
  S2.setClickTicks(250); // delay differentiating single clocks from double clicks
  S2.setDebounceTicks(10);// Prevent accidental double press

  S2.attachClick(S2_Press); // attach normal press to S2 (green button)
  S2.attachDuringLongPress(S2_long_press); // attach long press to S2 (green button)
  S2.attachDoubleClick(Check_brightness); // attach double press to S2 (green button)
  S2.attachLongPressStop(S2_long_press_stop); // called when long press stops
  // S3
  S3.setDebounceTicks(10);// Prevent accidental double press
  S3.setClickTicks(250); // delay differentiating single clocks from double clicks
  S3.attachClick(ON_OFF_Press);// attach normal press to ON_OFF_BUTTON
  S3.attachDoubleClick(WRITE_TO_EEPROM); // write current animation to memory
  S3.attachLongPressStart(Change_Direction); //change direction of the led strip

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
  FastLED.addLeds<LED_TYPE, LED_STRIP_PIN, COLOR_ORDER>(leds, LED_COUNT).setCorrection(TypicalLEDStrip); // GRB ordering is typical
  thisPalette = RainbowColors_p;
  thatPalette = OceanColors_p;
  currentBlending = LINEARBLEND;
  currentPalette = RainbowStripeColors_p;
  resetvars();
  X = Xorig;
  Y = Yorig; // Initialize the variables

  Startup_Animation();
  EEPROM_read();
}
//End Setup


void loop()
{
  if (!ON_OFF_STATE) // check if off state enabled
  {
    S3.tick();
    FastLED.setBrightness(0);
    FastLED.show();
    SEG_SHOW_OFF();
    return; // exit and run loop() again
  }
  previousMillis = millis();
  Animation_Tick();
  S1.tick(); // check status of button 1
  S2.tick(); // Check status of button 2
  S3.tick(); // Check status of button 3
  FastLED.show();
  FastLED.setBrightness(brightness);
  if (BRIGHTNESS_MODIFY_check)
  {
    Brightness_Check();
    BRIGHTNESS_MODIFY = true;
    return; // exit and run loop() again
  }
  BRIGHTNESS_MODIFY = false;
}
//End Loop

void S1_Press()
{
  EVERY_N_MILLISECONDS(button_delay)
  {
    previousanimation();
  }
}

void S2_long_press()
{
  BRIGHTNESS_MODIFY_check = true;
  EVERY_N_MILLISECONDS(50)
  {
    higher_brightness();
  }
}

void Change_Direction()
{
  Reverse_Direction = !Reverse_Direction;
}

void S1_long_press()
{
  BRIGHTNESS_MODIFY_check = true;
  EVERY_N_MILLISECONDS(50)
  {
    lower_brightness();
  }
}
void Check_brightness()
{
  BRIGHTNESS_MODIFY_check = !BRIGHTNESS_MODIFY_check;
}

void S2_Press()
{
  EVERY_N_MILLISECONDS(button_delay)
  {
    nextanimation();
  }
}

void WRITE_TO_EEPROM()
{
  EVERY_N_MILLISECONDS(button_delay)
  {
    EEPROM_write();
  }
}

void ON_OFF_Press()
{
  ON_OFF_STATE = !ON_OFF_STATE;
}
//End Button check

void S2_long_press_stop()
{
  BRIGHTNESS_MODIFY_check = false;
}

void S1_long_press_stop()
{
  BRIGHTNESS_MODIFY_check = false;
}

void previousanimation()
{
  Flash_Red();
  if (animation > 0)
  {
    animation -= 1;
    return; // exit when condition met and go to previous function
  }
  animation = animation_count - 1;
} //End Prevcious animation

void nextanimation()
{
  Flash_Green();
  if (animation < animation_count - 1)
  {
    animation += 1;
    return; // exit when condition met and go to previous function
  }
  animation = 0;
}
//End Next Animation

void SetupRandomColorPalette()
{

  targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));

} // SetupRandomColorPalette()

void SetupSimilarColorPalette()
{

  static uint8_t baseC = random8(); // You can use this as a baseline colour if you want similar hues in the next line, i.e. baseC+random8(0,5) for the hue.
  targetPalette = CRGBPalette16(CHSV(baseC + random8(0, 10), 255, random8(128, 255)), CHSV(baseC + random8(0, 10), 255, random8(128, 255)), CHSV(baseC + random8(0, 10), 192, random8(128, 255)), CHSV(baseC + random8(0, 10), 255, random8(128, 255)));

} // SetupSimilarColorPalette()

// RainbowColors_p, RainbowStripeColors_p, OceanColors_p, CloudColors_p, ForestColors_p, and PartyColors_p.
void ChangeMe()
{

  uint8_t secondHand = (millis() / 1000) % 60; // Increase this if you want a longer demo.
  static uint8_t lastSecond = 99;              // Static variable, means it's only defined once. This is our 'debounce' variable.
  if (lastSecond == secondHand)
  {
    return;
  }
  lastSecond = secondHand;
  switch (secondHand)
  {
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

} // ChangeMe()

void two_sin()
{
  if (Reverse_Direction)
  {
    thisdir ? thisphase += beatsin8(thisspeed, 2, 10) : thisphase += beatsin8(thisspeed, 2, 10);
    thatdir ? thatphase += beatsin8(thisspeed, 2, 10) : thatphase += beatsin8(thatspeed, 2, 10);
  } else {
    thisdir ? thisphase += beatsin8(thisspeed, 2, 10) : thisphase -= beatsin8(thisspeed, 2, 10);
    thatdir ? thatphase += beatsin8(thisspeed, 2, 10) : thatphase -= beatsin8(thatspeed, 2, 10);
  }
  thishue += thisrot; // Hue rotation is fun for thisSpeedy_Wave.
  thathue += thatrot; // It's also fun for thatSpeedy_Wave.
  for (int k = 0; k < LED_COUNT - 1; k++)
  {
    int thisbright = qsuba(cubicwave8((k * allfreq) + thisphase), thiscutoff);       // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    int thatbright = qsuba(cubicwave8((k * allfreq) + 128 + thatphase), thatcutoff); // This Speedy_Wave is 180 degrees out of phase (with the value of 128).

    leds[k] = ColorFromPalette(thisPalette, thishue, thisbright, currentBlending);
    leds[k] += ColorFromPalette(thatPalette, thathue, thatbright, currentBlending);
  }
  nscale8(leds, LED_COUNT, fadeval);

} // two_sin()

void Sine_Wave_V2()
{
  ChangeMe();

  EVERY_N_MILLISECONDS(thisdelay)
  {
    two_sin(); // Routine is still delay based, but at least it's now a non-blocking day.
  }
}
//End Double Sine_Wave_V1

void resetvars()
{ // Reset the variable back to the beginning.

  thishue = 0;      // You can change the starting hue value for the first Speedy_Wave.
  thathue = 140;    // You can change the starting hue for other Speedy_Wave.
  thisrot = 1;      // You can change how quickly the hue rotates for this Speedy_Wave. Currently 0.
  thatrot = 1;      // You can change how quickly the hue rotates for the other Speedy_Wave. Currently 0.
  allsat = 255;     // I like 'em fully saturated with colour.
  thisdir = 0;      // Change the direction of the first Speedy_Wave.
  thatdir = 0;      // Change the direction of the other Speedy_Wave.
  alldir = 0;       // You can change direction.
  thisspeed = 4;    // You can change the speed, and use negative values.
  thatspeed = 4;    // You can change the speed, and use negative values.
  allfreq = 32;     // You can change the frequency, thus overall width of bars.
  thisphase = 0;    // Phase change value gets calculated.
  thatphase = 0;    // Phase change value gets calculated.
  thiscutoff = 192; // You can change the cutoff value to display this Speedy_Wave. Lower value = longer Speedy_Wave.
  thatcutoff = 192; // You can change the cutoff value to display that Speedy_Wave. Lower value = longer Speedy_Wave.
  thisdelay = 10;   // You can change the delay. Also you can change the allspeed variable above.
  fadeval = 192;    // How quickly we fade.

} // resetvars()

void Rainbow_Palette()
{
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  Palette_filler(base_index,6);
  base_cycle(2);
} // Rainbow_Palette()

void Star_Night_Animation()
{

  EVERY_N_SECONDS(5)
  {
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

  index = (sin8(X) + cos8(Y)) / 2; // Guarantees maximum value of 255

  CRGB newcolor = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);

  //  nblend(leds[X%LED_COUNT-1], newcolor, 224);          // Try and smooth it out a bit. Higher # means less smoothing.
  nblend(leds[map(X, 0, 65535, 0, LED_COUNT)], newcolor, 224); // Try and smooth it out a bit. Higher # means less smoothing.

  fadeToBlackBy(leds, LED_COUNT, 16); // 8 bit, 1 = slow, 255 = fast

} // Star_Night_Animation()

void Star_Night()
{
  EVERY_N_MILLISECONDS(60)
  {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges); // Blend towards the target palette
  }

  EVERY_N_MILLISECONDS(50)
  {
    Star_Night_Animation();
  }
}
//End star Night

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{

  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, colorIndex + sin8(i * 16), 255);
    colorIndex += 3;
  }

} // FillLEDsFromPaletteColors()

void ChangePalettePeriodically()
{

  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if (lastSecond == secondHand)
  {
    return;
  }
  lastSecond = secondHand;
  CRGB p = CHSV(HUE_PURPLE, 255, 255);
  CRGB g = CHSV(HUE_GREEN, 255, 255);
  CRGB b = CRGB::Black;
  CRGB w = CRGB::White;
  switch (secondHand)
  {
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

} // ChangePalettePeriodically()

void Random_Palette_Crossfade()
{
  ChangePalettePeriodically();

  EVERY_N_MILLISECONDS(100)
  {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
  }

  EVERY_N_MILLISECONDS(thisdelay)
  {
    static uint8_t startIndex = 0;
    startIndex += 1; // motion speed
    FillLEDsFromPaletteColors(startIndex);
  }
}
//End Random Crossfade palette

void Running_Stripes()
{ // Eight colored dots, weaving in and out of sync with each other.

  fadeToBlackBy(leds, LED_COUNT, 20);
  byte dothue = 0;

  for (int i = 0; i < 8; i++)
  {
    leds[beatsin16(i + 7, 0, LED_COUNT - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }

} // Running_Stripes()

void Ocean_Wave()
{ // Colored stripes pulsing at a defined Beats-Per-Minute.

  uint8_t BeatsPerMinute = 62;
  currentBlending = LINEARBLEND;
  CRGBPalette16 palette = OceanColors_p; //RainbowStripeColors_p
  uint8_t beat = beat8(BeatsPerMinute, 64);//beatsin8(BeatsPerMinute, 64, 255) +

  for (int i = 0; i < LED_COUNT; i++)
  { //9948
    leds[i] = ColorFromPalette(palette, base_index + (i * 2), 255, currentBlending);
  }
  base_cycle(25);

} // Ocean_Wave()

void Palette_filler(uint8_t colorIndex,uint8_t speed_value)
{
  //Fills the leds with palette instead of pattern function

  for (int i = 0; i < LED_COUNT; ++i)
  {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += speed_value;
  }
} //Palette Filler

void Rainbow_Stripe_Palette()
{
  //Red white and blue colors
  currentPalette = RainbowStripeColors_p;
  currentBlending = LINEARBLEND;
  Palette_filler(base_index,3);
  base_cycle(25);
} //Red Whit Blue palette

void Palette_PG()
{
  //purple and green colors
  CRGB purple = CHSV(HUE_PURPLE, 255, 255);
  CRGB green = CHSV(HUE_GREEN, 255, 255);
  CRGB black = CRGB::Black;

  currentPalette = CRGBPalette16(
                     green, green, green, black,
                     purple, purple, purple, black,
                     green, green, green, black,
                     purple, purple, purple, black);
  currentBlending = LINEARBLEND;
  Palette_filler(base_index,3);
  base_cycle(25);
} //Purple and Green palette


void Palette_RP()
{
  //purple, blue and red colors
  CRGB blue = CHSV(HUE_BLUE, 255, 255);
  CRGB purple = CHSV(HUE_PURPLE, 255, 255);
  CRGB red = CHSV(HUE_RED, 255, 255);

  currentPalette = CRGBPalette16(
                     red, red, purple, blue,
                     blue, purple, red, red,
                     purple, blue, blue, purple,
                     red, red, purple, blue);
  currentBlending = LINEARBLEND;
  Palette_filler(base_index,3);
  base_cycle(25);
} //Purple, Blue and Red palette

void solid_rainbow()
{
  fill_solid(leds, LED_COUNT, CHSV(cos8(base_index), 255, 255));
  base_cycle(25);
} //solid rainbow

void rainbow_ish()
{ //Rainbowish animation with changing pattern
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88(87, 220, 250);
  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16; //base_index * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis;
  sLastMillis = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88(400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for (uint16_t i = 0; i < LED_COUNT; i++)
  {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16 += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV(hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (LED_COUNT - 1) - pixelnumber;

    nblend(leds[pixelnumber], newcolor, 64);
  }
  base_cycle(25);
} //Rainbow-ish

void Animation_Tick()
{
  segment_display();
  switch (animation)
  {
    case 0:
      solid_rainbow();
      break;
    case 1:
      Sine_Wave_V2();
      break;
    case 2:
      Rainbow_Palette();
      break;
    case 3:
      Star_Night();
      break;
    case 4:
      Random_Palette_Crossfade();
      break;
    case 5:
      Palette_PG();
      break;
    case 6:
      Running_Stripes();
      break;
    case 7:
      Ocean_Wave();
      break;
    case 8:
      rainbow_ish();
      break;
    case 9:
      Rainbow_Stripe_Palette();
      break;
    case 10:
      Palette_RP();
      break;
  }
}
//End Animation Tick

void Flash_Yellow()
{
  FastLED.setBrightness(25);
  Solid_Black_With_Delay();
  Solid_Yellow_With_Delay();
  Solid_Black_With_Delay();
  Solid_Yellow_With_Delay();
  Solid_Black_With_Delay();
  FastLED.setBrightness(brightness);
}

void Flash_Green()
{
  FastLED.setBrightness(25);
  Solid_Black_With_Delay();
  Solid_Green_With_Delay();
  Solid_Black_With_Delay();
  Solid_Green_With_Delay();
  Solid_Black_With_Delay();
  FastLED.setBrightness(brightness);
}

void Flash_Red()
{
  FastLED.setBrightness(25);
  Solid_Black_With_Delay();
  Solid_Red_With_Delay();
  Solid_Black_With_Delay();
  Solid_Red_With_Delay();
  Solid_Black_With_Delay();
  FastLED.setBrightness(brightness);
}

void Startup_Animation()
{
  forward();
  backward();
  delay(10);
  forward();
  backward();
}

void forward()
{
  for (int i = LED_COUNT / 2; i < LED_COUNT; i++)
  {
    leds[i] = CHSV(i * 6, 255, 255); //right
    leds[(i * -1) + LED_COUNT] = CHSV(i * 6, 255, 255); //left
    FastLED.show();
    delay(2);
  }
}
void backward()
{
  for (int i = LED_COUNT; i > LED_COUNT / 2; i--)
  {
    leds[i] = CHSV(0, 0, 0); //right
    leds[(i * -1) + LED_COUNT] = CHSV(0, 0, 0); //left
    FastLED.show();
    delay(2);
  }
}
//End Startup Animation

void Solid_Red_With_Delay()
{
  fill_solid(leds, LED_COUNT, CRGB::Red);
  FastLED.show();
  delay(75);
}

void Solid_Green_With_Delay()
{
  fill_solid(leds, LED_COUNT, CRGB::Green);
  FastLED.show();
  delay(75);
}

void Solid_Green_With_Long_Delay()
{
  fill_solid(leds, LED_COUNT, CRGB::Green);
  FastLED.show();
  delay(750);
}

void Solid_Black_With_Delay()
{
  fill_solid(leds, LED_COUNT, CRGB::Black);
  FastLED.show();
  delay(75);
}

void Solid_Yellow_With_Delay()
{
  fill_solid(leds, LED_COUNT, CRGB::Yellow);
  FastLED.show();
  delay(75);
}

void EEPROM_write()
{
  EEPROM.put(animation_mem_address, animation);
  EEPROM.put(brightness_mem_address, brightness);
  Flash_Yellow();
}

void EEPROM_read()
{
  EEPROM.get(animation_mem_address, animation);
  EEPROM.get(brightness_mem_address, brightness);
}

void base_cycle(uint8_t fastness)
{
  if (Reverse_Direction)
  {
    EVERY_N_MILLISECONDS(fastness)
    { // Speed that effects almost all animations
      base_index--;
    }
    return;
  }
  EVERY_N_MILLISECONDS(fastness)
  { // Speed that effects almost all animations
    base_index++;
  }

}

void lower_brightness()
{
  if (brightness > min_brightness)
  {
    brightness -= 1;
    return;
  }
  brightness = min_brightness;
}

void higher_brightness()
{
  if (brightness < max_brightness)
  {
    brightness += 1;
    return;
  }
  brightness = max_brightness;
}

void Brightness_Check()
{
  uint8_t brightness_number = brightness - 100;
  if ((brightness_number < 9999) && (brightness_number > 999))
  {
    display_segment(1, brightness_number % 10); // display brightness number on segment 1
    display_segment(2, brightness_number / 10 % 10); // display brightness number on segment 2
    display_segment(3, brightness_number / 100 % 10); // display brightness number on segment 3
    display_segment(4, brightness_number / 1000 % 10); // display brightness number on segment 4
    return;
  }
  if ((brightness_number < 999) && (brightness_number > 99))
  {
    display_segment(1, brightness_number % 10); // display brightness number on segment 1
    display_segment(2, brightness_number / 10 % 10); // display brightness number on segment 2
    display_segment(3, brightness_number / 100 % 10); // display brightness number on segment 3
    return;
  }
  if ((brightness_number < 99) && (brightness_number > 9))
  {
    display_segment(1, brightness_number % 10); // display brightness number on segment 1
    display_segment(2, brightness_number / 10 % 10); // display brightness number on segment 2
    return;
  }
  display_segment(1, brightness_number); // display brightness number on segment 1
}

void SEG_SHOW_OFF()
{
  if (millis() - previousMillis > off_interval) {
    LETTER_BLANK();
    return;
  }
  display_segment(3, 0); // display number 0 on segment 3
  display_segment(2, 15); // display letter f on segment 2
  display_segment(1, 15); // display letter f on segment 1
}

void NUMBER_0()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_8()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_9()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_7()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_6()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_5()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_4()
{
  digitalWrite(SEG_A, LOW);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_3()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_2()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, LOW);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}
void NUMBER_1()
{
  digitalWrite(SEG_A, LOW);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DP, LOW);
}
void LETTER_F()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, LOW);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_DP, LOW);
}

void LETTER_N()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DP, LOW);
}

void LETTER_A()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}
void LETTER_I_DP()
{
  digitalWrite(SEG_A, LOW);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_DP, HIGH);
}

void LETTER_BLANK()
{
  digitalWrite(SEG_A, LOW);
  digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, LOW);
  digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DP, LOW);
}

void LETTER_J()
{
  digitalWrite(SEG_A, LOW);
  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, LOW);
  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DP, LOW);
}

void LETTER_C()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, LOW);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, LOW);
  digitalWrite(SEG_DP, LOW);
}

void LETTER_E()
{
  digitalWrite(SEG_A, HIGH);
  digitalWrite(SEG_B, LOW);
  digitalWrite(SEG_C, LOW);
  digitalWrite(SEG_D, HIGH);
  digitalWrite(SEG_E, HIGH);
  digitalWrite(SEG_F, HIGH);
  digitalWrite(SEG_G, HIGH);
  digitalWrite(SEG_DP, LOW);
}

void display_segment(uint8_t dig, uint8_t number)
{
  switch (dig)
  {
    case 1:
      digitalWrite(DIG_1, LOW);
      digitalWrite(DIG_2, HIGH);
      digitalWrite(DIG_3, HIGH);
      digitalWrite(DIG_4, HIGH);
      break;
    case 2:
      digitalWrite(DIG_1, HIGH);
      digitalWrite(DIG_2, LOW);
      digitalWrite(DIG_3, HIGH);
      digitalWrite(DIG_4, HIGH);
      break;
    case 3:
      digitalWrite(DIG_1, HIGH);
      digitalWrite(DIG_2, HIGH);
      digitalWrite(DIG_3, LOW);
      digitalWrite(DIG_4, HIGH);
      break;
    case 4:
      digitalWrite(DIG_1, HIGH);
      digitalWrite(DIG_2, HIGH);
      digitalWrite(DIG_3, HIGH);
      digitalWrite(DIG_4, LOW);
      break;
  }
  switch (number)
  {
    case 1:
      NUMBER_1();
      break;
    case 2:
      NUMBER_2();
      break;
    case 3:
      NUMBER_3();
      break;
    case 4:
      NUMBER_4();
      break;
    case 5:
      NUMBER_5();
      break;
    case 6:
      NUMBER_6();
      break;
    case 7:
      NUMBER_7();
      break;
    case 8:
      NUMBER_8();
      break;
    case 9:
      NUMBER_9();
      break;
    case 0:
      NUMBER_0();
      break;
    case 10:
      LETTER_A();
      break;
    case 11:
      NUMBER_8();
      break;
    case 12:
      LETTER_C();
      break;
    case 13:
      NUMBER_0();
      break;
    case 14:
      LETTER_E();
      break;
    case 15:
      LETTER_F();
      break;
  }
  delay(1);
  LETTER_BLANK();
  delay(1);
} // End segment display

void segment_display()
{
  uint8_t animation_number = animation + 1;
  if (BRIGHTNESS_MODIFY)
  {
    delay(1);
    LETTER_BLANK();
    return;
  }
  if ((animation_number < 9999) && (animation_number > 999))
  {
    display_segment(1, animation_number % 10); // display animation number on segment 1
    display_segment(2, animation_number / 10 % 10); // display animation number on segment 2
    display_segment(3, animation_number / 100 % 10); // display animation number on segment 3
    display_segment(4, animation_number / 1000 % 10); // display animation number on segment 4
    return;
  }
  if ((animation_number < 999) && (animation_number > 99))
  {
    display_segment(1, animation_number % 10); // display animation number on segment 1
    display_segment(2, animation_number / 10 % 10); // display animation number on segment 2
    display_segment(3, animation_number / 100 % 10); // display animation number on segment 3
    return;
  }
  if ((animation_number < 99) && (animation_number > 9))
  {
    display_segment(1, animation_number % 10); // display animation number on segment 1
    display_segment(2, animation_number / 10 % 10); // display animation number on segment 2
    return;
  }
  display_segment(1, animation_number); // display animation number on segment 1
}
