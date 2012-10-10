/* Keyboard example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

uint8_t keyState[8];

/** Pin assignments **/
/* Keyboard pin  |  Atmega pin
 * --------------|------------
 *       1       |    B0
 *       2       |    B1
 *       3       |    B2
 *       4       |    vcc
 *       5       |    gnd
 *       6       |    B3
 *       7       |    B7
 *       8       |    D0
 *       9       |    D1
 *       10      |    D2
 *       11      |    D3
 *       12      |    C6
 *       13      |    C7
 *       14      |    D5
 *       15      |    D4
 *       16      |    D6
 *       17      |    D7
 *       18      |    B4
 *       19      |    B5
 *       20      |    B6
 *       21      |    F7
 *       22      |    F6
 *       23      |    F5
 *       24      |    F4
 */

/** Keyboard matrix layout
 *  ----------------------
 *    S1   0   1   2   3   4   5   6
 * S2     B7  C6  D5  D4  D6  B4  B6
 *      +---------------------------
 * 0 D0 | 57  53  XX  24  34   7  17
 * 1 D7 | 14   1  30  16  23   9  21
 * 2 B5 | 28  10  XX  25  35   8  45
 * 3 D3 | 27  51  XX  38  32   4  20  
 * 4 D2 | 26  13  39  37  46   3  31
 * 5 F7 | 15  52  XX  49  33   6  47
 * 6 C7 | 40  12  XX  50  18   5  22
 * 7 D1 | 44  41  11  36  48   2  19
 *
 */
int8_t kbMatrix[7*8] = {
  57, 53, -1, 24, 34,  7, 17,
  14,  1, 30, 16, 23,  9, 21,
  28, 10, -1, 25, 35,  8, 45,
  27, 51, -1, 38, 32,  4, 20,
  26, 13, 39, 37, 46,  3, 31,
  15, 52, -1, 49, 33,  6, 47,
  40, 12, -1, 50, 18,  5, 22,
  44, 41, 11, 36, 48,  2, 19
};

int charMap[59] = {
  -1, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
  KEY_0, -1, KEY_MINUS, -1, -1, KEY_ESC, KEY_Q, KEY_W, KEY_E, KEY_R,
  KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, -1, KEY_ENTER, -1, -1,
  KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
  KEY_BACKSPACE, -1, -1, -1, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N,
  KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, -1, -1, -1, KEY_SPACE, -1 };
 
int8_t getKey(uint8_t s1, uint8_t s2) {
  return kbMatrix[(s2*7)+s1];
}

#define OUTPUT(port,pin) { DDR##port |= _BV(pin); }
#define INPUT(port,pin) { DDR##port &= ~_BV(pin); }
#define SET_PIN(port,pin) { PORT##port |= _BV(pin); }
#define CLR_PIN(port,pin) { PORT##port &= ~_BV(pin); }
#define READ_PIN(port,pin) ((PIN##port & _BV(pin)) != 0)


void initMatrixPins(void) {
  // S1 are outputs; S2 are inputs
  OUTPUT(B,7); OUTPUT(C,6); OUTPUT(D,5); OUTPUT(D,4);
  OUTPUT(D,6); OUTPUT(B,4); OUTPUT(B,6);
  INPUT(D,0); INPUT(D,7); INPUT(B,5); INPUT(D,3);
  INPUT(D,2); INPUT(F,7); INPUT(C,7); INPUT(D,1);
  // Turn on pullups
  SET_PIN(D,0); SET_PIN(D,7); SET_PIN(B,5); SET_PIN(D,3);
  SET_PIN(D,2); SET_PIN(F,7); SET_PIN(C,7); SET_PIN(D,1);
}

void initSwitchPins(void) {
  INPUT(B,1); INPUT(B,2); INPUT(B,3);
  INPUT(F,6); INPUT(F,5); INPUT(F,4);
  // Turn on pullups
  SET_PIN(B,1); SET_PIN(B,2); SET_PIN(B,3);
  SET_PIN(F,6); SET_PIN(F,5); SET_PIN(F,4);
}

void initLEDPin(void) {
  OUTPUT(B,0);
  // Turn off LED
  SET_PIN(B,0);
}

uint8_t key_count;
uint8_t key_touched;

void init(void) {
  initMatrixPins();
  initSwitchPins();
  initLEDPin();
  key_count = 0;
  key_touched = 0;
}

void clearReport(void) {
  keyboard_modifier_keys = 0;
  for (int8_t i = 0; i < 6; i++) {
    keyboard_keys[i] = 0;
  }
  key_count = 0;
  key_touched = 0;
}

void addToReport(uint16_t key) {
  key_touched = 1;
  if (key > 0xff) {
    keyboard_modifier_keys |= key >> 8;
  } else {
    if (key_count < 6) {
      keyboard_keys[key_count++] = key & 0xff;
    }
  }
}

void addModifier(uint16_t mod) {
  keyboard_modifier_keys |= mod;
}

void endReport(void) {
  usb_keyboard_send();
}

void typeNum(uint8_t num) {
  num = num % 100;
  if (num > 10) {
    typeNum(num/10);
    num = num % 10;
  }
  if (num == 0) {
    usb_keyboard_press(KEY_0, 0);
  } else {
    usb_keyboard_press(KEY_1 + num - 1, 0);
  }
}

void scanModifiers() {
  if (!READ_PIN(F,6)) {
    addModifier(KEY_CTRL);
  }
  if (!READ_PIN(F,5)) {
    addModifier(KEY_SHIFT);
  }
}

void scanLine(uint8_t s1) {
  uint8_t line = 
    (READ_PIN(D,0)?(1<<0):0) |
    (READ_PIN(D,7)?(1<<1):0) |
    (READ_PIN(B,5)?(1<<2):0) |
    (READ_PIN(D,3)?(1<<3):0) |
    (READ_PIN(D,2)?(1<<4):0) |
    (READ_PIN(F,7)?(1<<5):0) |
    (READ_PIN(C,7)?(1<<6):0) |
    (READ_PIN(D,1)?(1<<7):0);

  for (uint8_t s2 = 0; s2 < 8; s2++) {
    if ((line & _BV(s2)) == 0) {
      int8_t key = getKey(s1,s2);
      if (key != -1) {
	int mapped = charMap[key];
	addToReport(mapped);
      }
    }
  }
}

void scanMatrix(void) {
  CLR_PIN(B,7); scanLine(0); SET_PIN(B,7);
  _delay_ms(1);
  CLR_PIN(C,6); scanLine(1); SET_PIN(C,6);
  _delay_ms(1);
  CLR_PIN(D,5); scanLine(2); SET_PIN(D,5);
  _delay_ms(1);
  CLR_PIN(D,4); scanLine(3); SET_PIN(D,4);
  _delay_ms(1);
  CLR_PIN(D,6); scanLine(4); SET_PIN(D,6);
  _delay_ms(1);
  CLR_PIN(B,4); scanLine(5); SET_PIN(B,4);
  _delay_ms(1);
  CLR_PIN(B,6); scanLine(6); SET_PIN(B,6);  
  _delay_ms(1);
}

int main(void)
{
	// set for 16 MHz clock
	CPU_PRESCALE(0);

	init();
	TCCR0A &= 0x03;
	TCCR1A &= 0x03;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);
	
	while (1) {
	  clearReport();
	  scanModifiers();
	  scanMatrix();
	  endReport();
	  // debouncing delay
	  _delay_ms(10);
	}
}


