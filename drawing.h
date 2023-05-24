#include "font_types.h"

#ifndef DRAWING_H
#define DRAWING_H

/*
*  draw pixel of color to x,y
*/
void draw_pixel(int x, int y, unsigned short color, unsigned short *fb);

/*
*  draws square of size width*height of given color
*/
void draw_square(int startX, int startY, int width, int height, unsigned short color, unsigned short *fb);

/*
*  draws hollow square of size width*height of given color and thickness
*/
void draw_hollow_square(int startX, int startY, int width, int height, int thick, unsigned short color, unsigned short *fb);

/*
*  print one character by 8x16 font
*/
void print_char(char c, int x, int y, int scale, unsigned short color, unsigned short *fb);

/*
* print string by proporcional font
*/
void print_string(char text[], int len, int x, int y, int scale, unsigned short color, unsigned short *fb);

#endif