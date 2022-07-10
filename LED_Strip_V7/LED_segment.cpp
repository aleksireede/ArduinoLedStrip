#include "Arduino.h"
#include "LED_segment.h"

void display_segment(uint8_t dig, uint8_t number)
{
  switch (dig)
  {
    case 1: // First digit from right
      digit_activate(0, 1, 1, 1);
      break;
    case 2: // Second digit from right
      digit_activate(1, 0, 1, 1);
      break;
    case 3: // Third digit from right
      digit_activate(1, 1, 0, 1);
      break;
    case 4: // Fourth digit from right
      digit_activate(1, 1, 1, 0);
      break;
  }
  switch (number)
  {
    case 0: // 0
      segment_activate(1, 1, 1, 1, 1, 1, 0, 0);
      break;
    case 1: // 1
      segment_activate(0, 1, 1, 0, 0, 0, 0, 0);
      break;
    case 2: // 2
      segment_activate(1, 1, 0, 1, 1, 0, 1, 0);
      break;
    case 3: // 3
      segment_activate(1, 1, 1, 1, 0, 0, 1, 0);
      break;
    case 4: // 4
      segment_activate(0, 1, 1, 0, 0, 1, 1, 0);
      break;
    case 5: // 5
      segment_activate(1, 0, 1, 1, 0, 1, 1, 0);
      break;
    case 6: // 6
      segment_activate(1, 0, 1, 1, 1, 1, 1, 0);
      break;
    case 7: // 7
      segment_activate(1, 1, 1, 0, 0, 0, 0, 0);
      break;
    case 8: // 8
      segment_activate(1, 1, 1, 1, 1, 1, 1, 0);
      break;
    case 9: // 9
      segment_activate(1, 1, 1, 1, 0, 1, 1, 0);
      break;
    case 10: // A
      segment_activate(1, 1, 1, 0, 1, 1, 1, 0);
      break;
    case 11: // B
      segment_activate(1, 1, 1, 1, 1, 1, 1, 0);
      break;
    case 12: // C
      segment_activate(1, 0, 0, 1, 1, 1, 0, 0);
      break;
    case 13: // D
      segment_activate(1, 1, 1, 1, 1, 1, 0, 0);
      break;
    case 14: // E
      segment_activate(1, 0, 0, 1, 1, 1, 1, 0);
      break;
    case 15: // F
      segment_activate(1, 0, 0, 0, 1, 1, 1, 0);
      break;
    case 16: // Decimal point
      segment_activate(0, 0, 0, 0, 0, 0, 0, 1);
  }
  delay(1);
  segment_activate(0, 0, 0, 0, 0, 0, 0, 0);
  delay(1);
} // End segment display

void segment_activate(bool A, bool B, bool C, bool D, bool E, bool F, bool G, bool H)
{
  digitalWrite(SEG_A, A);
  digitalWrite(SEG_B, B);
  digitalWrite(SEG_C, C);
  digitalWrite(SEG_D, D);
  digitalWrite(SEG_E, E);
  digitalWrite(SEG_F, F);
  digitalWrite(SEG_G, G);
  digitalWrite(SEG_DP, H);
}
void digit_activate(bool dig1, bool dig2, bool dig3, bool dig4)
{
  digitalWrite(DIG_1, dig1);
  digitalWrite(DIG_2, dig2);
  digitalWrite(DIG_3, dig3);
  digitalWrite(DIG_4, dig4);
}


void Led_segment_show(uint16_t number)
{
  display_segment(1, number % 10); // display brightness number on segment 1
  display_segment(2, number / 10 % 10); // display brightness number on segment 2
  display_segment(3, number / 100 % 10); // display brightness number on segment 3
  display_segment(4, number / 1000 % 10); // display brightness number on segment 4
}
