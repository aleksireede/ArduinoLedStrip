#ifndef LED_segment_h
#define LED_segment_h

#include "Arduino.h"

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

void display_segment(uint8_t dig, uint8_t number);
void segment_activate(bool A, bool B, bool C, bool D, bool E, bool F, bool G, bool H);
void digit_activate(bool dig1, bool dig2, bool dig3, bool dig4);
void Led_segment_show(uint16_t number);

#endif
