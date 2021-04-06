#include <FastLED.h>
#include <OneButton.h>
#include <EEPROM.h>
//Inlcude libraries

#define LED_COUNT 88
struct CRGB leds[LED_COUNT];
struct CRGB leds2[LED_COUNT];
struct CRGB leds3[LED_COUNT];
#define LED_STRIP_PIN 3
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
uint8_t Brightness = 255;
//End Led Strip

bool ON = true;
#define ON_OFF_PIN 4 //button

// Initialize changeable global variables. Play around with these!!!
uint8_t thisrot = 1;      // You can change how quickly the hue rotates for this Speedy_Wave.
int8_t thisspeed = 8;     // You can change the speed of the Speedy_Wave, and use negative values.
uint8_t allfreq = 32;     // You can change the frequency, thus distance between bars.
int thisphase = 0;        // Phase change value gets calculated.
uint8_t thiscutoff = 128; // You can change the cutoff value to display this Speedy_Wave. Lower value = longer Speedy_Wave.
uint8_t thisdelay = 30;   // You can change the delay. Also you can change the allspeed variable above.
uint8_t bgclr = 0;        // A rotating background colour.
uint8_t bgbright = 0;     // Brightness of background colour
uint8_t bgclrinc = 0;
int startIndex = 0;
CRGBPalette16 thisPalette;
CRGBPalette16 thatPalette;
TBlendType currentBlending; // NOBLEND or LINEARBLEND
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
uint8_t maxChanges = 24; // Value for blending between palettes.
uint8_t gHue = 0;
int MAX_INT_VALUE = 65536;

OneButton ON_Button(ON_OFF_PIN, true); //Attach on_off button to Onebutton function
int animation = 2;
int animation_count = 18;
int rom_value = 0;
int eeAddress = 0;   //Location we want the data to be put.
//End Variables and includes

void setup()
{
  delay(1000);
  Serial.begin(9600);
  ON_Button.attachDoubleClick(ON_doubleclick); // link the function to be called on a doubleclick event.
  ON_Button.attachClick(ON_singleclick);       // link the function to be called on a singleclick event.
  ON_Button.attachLongPressStop(ON_longclick); // link the function to be called on a longpress event.
  FastLED.setBrightness(Brightness);
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
  Animation_Tick();
  ON_Button.tick(); // check the status of the button
  FastLED.show();
}
//End Loop

void ON_singleclick()
{ // what happens when the button is clicked
  Serial.println("Press ON/OFF");
  delay(100);
  Flash_Green();
  nextanimation();
}
void ON_doubleclick()
{ // what happens when button is double-clicked
  Serial.println("Double Click ON/OFF");
  delay(100);
  Flash_Red();
  previousanimation();
}
void ON_longclick()
{ // what happens when button is long-pressed
  Serial.println("Long Press ON/OFF");
  delay(100);
  EEPROM_write();
}
//End Button check

void previousanimation()
{
  if (animation > 0)
  {
    animation -= 1;
  }
  else
  {
    animation = animation_count - 1;
  }
} //End Prevcious animation

void nextanimation()
{
  if (animation < animation_count - 1)
  {
    animation += 1;
  }
  else
  {
    animation = 0;
  }
}
//End Next Animation

void one_sine_pal(uint8_t colorIndex)
{ // This is the heart of this program. Sure is short.

  thisphase += thisspeed; // You can change direction and speed individually.

  for (int k = 0; k < LED_COUNT; k++)
  {                                                                            // For each of the LED's in the strand, set a Brightness based on a Speedy_Wave as follows:
    int thisbright = qsuba(cubicwave8((k * allfreq) + thisphase), thiscutoff); // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    leds[k] = CHSV(bgclr, 255, bgbright);                                      // First set a background colour, but fully saturated.
    leds[k] += ColorFromPalette(currentPalette, colorIndex + k, thisbright, currentBlending);
    colorIndex += thisrot;
  }

  bgclr += bgclrinc; // You can change the background colour or remove this and leave it fixed.

} // one_sine_pal()

void Cahange_Sine()
{

  uint8_t secondHand = (millis() / 1000) % 60; // Increase this if you want a longer demo.
  static uint8_t lastSecond = 99;              // Static variable, means it's only defined once. This is our 'debounce' variable.
  if (lastSecond != secondHand)
  {
    lastSecond = secondHand;
    switch (secondHand)
    {
    case 0:
      break;
    case 5:
      targetPalette = RainbowColors_p;
      bgclr = 0;
      bgbright = 32;
      bgclrinc = 1;
      thisrot = 1;
      thisdelay = 10;
      thisspeed = -4;
      allfreq = 16;
      thiscutoff = 128;
      currentBlending = NOBLEND;
      break;
    case 10:
      targetPalette = ForestColors_p;
      thisspeed = 12;
      thisrot = 0;
      thisspeed = 12;
      thisdelay = 10;
      thiscutoff = 128;
      allfreq = 64;
      bgclr = 50;
      bgbright = 15;
      currentBlending = LINEARBLEND;
      break;
    case 15:
      SetupRandomColorPalette();
      thisrot = 2;
      thisspeed = 16;
      break;
    case 20:
      targetPalette = LavaColors_p;
      thisspeed = 4;
      break;
      allfreq = 16;
      bgclr = 50;
      break;
    case 25:
      thiscutoff = 96;
      thisspeed = -4;
      thiscutoff = 224;
      bgclr = 20;
      bgbright = 8;
      break;
    case 30:
      SetupRandomColorPalette();
      thiscutoff = 96;
      thisrot = 1;
      break;
    case 35:
      targetPalette = OceanColors_p;
      break;
    case 40:
      SetupSimilarColorPalette();
      thiscutoff = 128;
      break;
    case 45:
      SetupSimilarColorPalette();
      thisspeed = 3;
      break;
    case 50:
      targetPalette = PartyColors_p;
      break;
    case 55:
      break;
    case 60:
      break;
    }
  }

} // Cahange_Sine()

void SetupRandomColorPalette()
{

  targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));

} // SetupRandomColorPalette()

void SetupSimilarColorPalette()
{

  static uint8_t baseC = random8(); // You can use this as a baseline colour if you want similar hues in the next line, i.e. baseC+random8(0,5) for the hue.
  targetPalette = CRGBPalette16(CHSV(baseC + random8(0, 10), 255, random8(128, 255)), CHSV(baseC + random8(0, 10), 255, random8(128, 255)), CHSV(baseC + random8(0, 10), 192, random8(128, 255)), CHSV(baseC + random8(0, 10), 255, random8(128, 255)));

} // SetupSimilarColorPalette()

void Sine_Wave_V1()
{
  Cahange_Sine();

  EVERY_N_MILLISECONDS(100)
  {
    uint8_t maxChanges = 24;
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges); // AWESOME palette blending capability.
  }

  EVERY_N_MILLISECONDS(thisdelay)
  {                        // FastLED based non-blocking delay to update/display the sequence.
    startIndex += thisrot; // Motion speed
    one_sine_pal(startIndex);
  }
}
//End Sine_Wave_V1

// RainbowColors_p, RainbowStripeColors_p, OceanColors_p, CloudColors_p, ForestColors_p, and PartyColors_p.
void ChangeMe()
{

  uint8_t secondHand = (millis() / 1000) % 60; // Increase this if you want a longer demo.
  static uint8_t lastSecond = 99;              // Static variable, means it's only defined once. This is our 'debounce' variable.

  if (lastSecond != secondHand)
  {
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
  }

} // ChangeMe()

void two_sin()
{

  thisdir ? thisphase += beatsin8(thisspeed, 2, 10) : thisphase -= beatsin8(thisspeed, 2, 10);
  thatdir ? thatphase += beatsin8(thisspeed, 2, 10) : thatphase -= beatsin8(thatspeed, 2, 10);
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

void Red_Stripe_Animation()
{ // running red stripe.

  for (int i = 0; i < LED_COUNT; i++)
  {
    uint8_t red = (millis() / 10) + (i * 12); // speed, length
    if (red > 128)
      red = 0;
    leds2[i] = CRGB(red, 0, red);
  }
} // Red_Stripe_Animation()

void Green_Stripe_Animation()
{ // running green stripe in opposite direction.
  for (int i = 0; i < LED_COUNT; i++)
  {
    uint8_t blue = (millis() / 5) - (i * 12); // speed, length
    if (blue > 128)
      blue = 0;
    leds3[i] = CRGB(0, 0, blue);
  }
} // Green_Stripe_Animation()

void Two_Opposite_Waves()
{
  Red_Stripe_Animation();   // render the first animation into leds2
  Green_Stripe_Animation(); // render the second animation into leds3

  uint8_t ratio = beatsin8(2); // Alternate between 0 and 255 every minute

  for (int i = 0; i < LED_COUNT; i++)
  { // mix the 2 arrays together
    leds[i] = blend(leds2[i], leds3[i], ratio);
  }
}

void Sine_Rainbow(uint8_t thisdelay, uint8_t deltahue)
{ // The fill_rainbow call doesn't support Brightness levels.

  //uint8_t thishue = millis()*(255-thisdelay)/255;             // To change the rate, add a beat or something to the result. 'thisdelay' must be a fixed value.

  thishue = beat8(50) + beatsin8(50, 0, 255); // This uses a FastLED sawtooth generator. Again, the '50' should not change on the fly.
                                              //thishue = beatsin8(50,0,255);                              // This can change speeds on the fly. You can also add these to each other.

  fill_rainbow(leds, LED_COUNT, thishue, deltahue); // Use FastLED's fill_rainbow routine.

} // Sine_Rainbow()

void Speedy_Wave()
{
  beatSpeedy_Wave();

  EVERY_N_MILLISECONDS(100)
  {
    uint8_t maxChanges = 24;
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges); // AWESOME palette blending capability.
  }

  EVERY_N_SECONDS(5)
  { // Change the target palette to a random one every 5 seconds.
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
  }
}

void beatSpeedy_Wave()
{

  uint8_t Speedy_Wave1 = beatsin8(9, 0, 255); // That's the same as beatsin8(9);
  uint8_t Speedy_Wave2 = beatsin8(8, 0, 255);
  uint8_t Speedy_Wave3 = beatsin8(7, 0, 255);
  uint8_t Speedy_Wave4 = beatsin8(6, 0, 255);

  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, i + Speedy_Wave1 + Speedy_Wave2 + Speedy_Wave3 + Speedy_Wave4, 255, currentBlending);
  }

} // beatSpeedy_Wave()

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

  if (lastSecond != secondHand)
  {
    lastSecond = secondHand;
    CRGB p = CHSV(HUE_PURPLE, 255, 255);
    CRGB g = CHSV(HUE_GREEN, 255, 255);
    CRGB b = CRGB::Black;
    CRGB w = CRGB::White;
    if (secondHand == 0)
    {
      targetPalette = RainbowColors_p;
    }
    if (secondHand == 10)
    {
      targetPalette = CRGBPalette16(g, g, b, b, p, p, b, b, g, g, b, b, p, p, b, b);
    }
    if (secondHand == 20)
    {
      targetPalette = CRGBPalette16(b, b, b, w, b, b, b, w, b, b, b, w, b, b, b, w);
    }
    if (secondHand == 30)
    {
      targetPalette = LavaColors_p;
    }
    if (secondHand == 40)
    {
      targetPalette = CloudColors_p;
    }
    if (secondHand == 50)
    {
      targetPalette = PartyColors_p;
    }
  }

} // ChangePalettePeriodically()

void Random_Palette_Crossfade()
{
  ChangePalettePeriodically();

  EVERY_N_MILLISECONDS(100)
  {
    uint8_t maxChanges = 28;
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

void animationA()
{ // running red stripe.
  for (int i = 0; i < LED_COUNT; i++)
  {
    uint8_t red = (millis() / 5) + (i * 12); // speed, length
    if (red > 128)
      red = 0;
    leds[i] = ColorFromPalette(currentPalette, red, red, currentBlending);
  }
} // animationA()

void Random_Wave()
{
  EVERY_N_MILLISECONDS(100)
  {
    uint8_t maxChanges = 24;
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges); // AWESOME palette blending capability.
  }

  EVERY_N_SECONDS(5)
  {                                   // Change the target palette to a random one every 5 seconds.
    static uint8_t baseC = random8(); // You can use this as a baseline colour if you want similar hues in the next line.
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
  }

  animationA();
}
//End Random Wave

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
  CRGBPalette16 palette = OceanColors_p; //RainbowStripeColors_p
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255) + beat8(BeatsPerMinute, 64);

  for (int i = 0; i < LED_COUNT; i++)
  { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
  EVERY_N_MILLISECONDS(20)
  { // slowly cycle the "base color" through the rainbow
    gHue++;
  }

} // Ocean_Wave()

void Palette_filler(uint8_t colorIndex)
{
  //Fills the leds with palette instead of pattern function
  uint8_t brightness = 255;

  for (int i = 0; i < LED_COUNT; ++i)
  {
    leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
} //Palette Filler

void Rainbow_Stripe_Palette()
{
  //Red white and blue colors
  currentPalette = RainbowStripeColors_p;
  currentBlending = LINEARBLEND;
  Palette_filler(gHue);
  EVERY_N_MILLISECONDS(20)
  { // slowly cycle the "base color" through the rainbow
    gHue++;
  }
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
  Palette_filler(gHue);
  EVERY_N_MILLISECONDS(20)
  { // slowly cycle the "base color" through the rainbow
    gHue++;
  }
} //Purple and Green palette

void solid_white()
{
  fill_solid(leds, LED_COUNT, CRGB::White);
} //solid white

void solid_red()
{
  fill_solid(leds, LED_COUNT, CRGB::Red);
} //solid red

void solid_green()
{
  fill_solid(leds, LED_COUNT, CRGB::Green);
} //solid green

void solid_blue()
{
  fill_solid(leds, LED_COUNT, CRGB::Blue);
} //solid blue

void rainbow()
{
  fill_rainbow(leds, LED_COUNT, gHue, 7);
  EVERY_N_MILLISECONDS(20)
  { // slowly cycle the "base color" through the rainbow
    gHue++;
  }
} //rainbow

void solid_rainbow()
{
  fill_solid(leds, LED_COUNT, CHSV(gHue, 255, 255));
  EVERY_N_MILLISECONDS(20)
  { // slowly cycle the "base color" through the rainbow
    gHue++;
  }
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

  uint16_t hue16 = sHue16; //gHue * 256;
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
  EVERY_N_MILLISECONDS(20)
  { // slowly cycle the "base color" through the rainbow
    gHue++;
  }
} //Rainbow-ish

void Animation_Tick()
{
  switch (animation)
  {
  case 0:
    Sine_Wave_V1();
    break;
  case 1:
    Two_Opposite_Waves();
    break;
  case 2:
    Sine_Wave_V2();
    break;
  case 3:
    Sine_Rainbow(200, 10);
    break;
  case 4:
    Speedy_Wave();
    break;
  case 5:
    Star_Night();
    break;
  case 6:
    Random_Palette_Crossfade();
    break;
  case 7:
    Random_Wave();
    break;
  case 8:
    Running_Stripes();
    break;
  case 9:
    Ocean_Wave();
    break;
  case 10:
    solid_white();
    break;
  case 11:
    solid_red();
    break;
  case 12:
    solid_green();
    break;
  case 13:
    solid_blue();
    break;
  case 14:
    rainbow_ish();
    break;
  case 15:
    Rainbow_Stripe_Palette();
    break;
  case 16:
    Palette_PG();
    break;
  case 17:
    solid_rainbow();
    break;
  }
}
//End Animation Tick

void Flash_Yellow()
{
  FastLED.setBrightness(Brightness / 6);
  Solid_Black_With_Delay();
  Solid_Yellow_With_Delay();
  Solid_Black_With_Delay();
  Solid_Yellow_With_Delay();
  Solid_Black_With_Delay();
  FastLED.setBrightness(Brightness);
}

void Flash_Green()
{
  FastLED.setBrightness(Brightness / 6);
  Solid_Black_With_Delay();
  Solid_Green_With_Delay();
  Solid_Black_With_Delay();
  Solid_Green_With_Delay();
  Solid_Black_With_Delay();
  FastLED.setBrightness(Brightness);
}

void Flash_Red()
{
  FastLED.setBrightness(Brightness / 6);
  Solid_Black_With_Delay();
  Solid_Red_With_Delay();
  Solid_Black_With_Delay();
  Solid_Red_With_Delay();
  Solid_Black_With_Delay();
  FastLED.setBrightness(Brightness);
}

void Startup_Animation()
{
  FastLED.setBrightness(Brightness / 6);
  Solid_Black_With_Delay();
  Solid_Red_With_Delay();
  Solid_Black_With_Delay();
  Solid_Red_With_Delay();
  Solid_Black_With_Delay();
  Solid_Green_With_Long_Delay();
  Solid_Black_With_Delay();
  FastLED.setBrightness(Brightness);
}
//End Startup Animation

void Solid_Red_With_Delay()
{
  fill_solid(leds, LED_COUNT, CRGB::Red);
  FastLED.show();
  delay(100);
}

void Solid_Green_With_Delay()
{
  fill_solid(leds, LED_COUNT, CRGB::Green);
  FastLED.show();
  delay(100);
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
  delay(100);
}

void Solid_Yellow_With_Delay()
{
  fill_solid(leds, LED_COUNT, CRGB::Yellow);
  FastLED.show();
  delay(100);
}

void EEPROM_write() 
{
  rom_value = animation;
  EEPROM.put(eeAddress, rom_value);
  Flash_Yellow();
}

void EEPROM_read() 
{
  EEPROM.get(eeAddress, rom_value);
  animation = rom_value;
}
