#include "mzapo_regs.h"

#ifndef HARDWARE_H
#define HARDWARE_H

/*
* control for 1st RGB LED
*/
void set_rgb1(int val, unsigned char *mem_base);

/*
*  control for 2nd RGB LED
*/
void set_rgb2(int val, unsigned char *mem_base);

/*
*  control for led stripe
*/
void set_leds(int val, unsigned char *mem_base);

/*
*  console print ready
*/
int pready();

/*
*  console print
*/
void pchar(char c);

/*
*  console get ready
*/
int gready();

/*
*  console get
*/
char gchar(void);


/*
*  reverse bit order of 32 bit number (for LED stripe)
*/
unsigned int reverseBits(unsigned int num);

#endif /*HARDWARE_H*/