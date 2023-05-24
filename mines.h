#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "queue.h"
#include "drawing.h"

#ifndef MINES_H
#define MINES_H

typedef struct
{
    int *data; //0 - 8 bomb count, -1 bomb
    int *revealed; //0 - hidden, 1 - shown, 2- flag
    int selectedX;
    int selectedY;
    int score;
} mines;

/*
* generate already allocated field
*/
void generate(mines *m);

/*
* reveal square on idx or adds inself to que
*/
void reveal_part(mines *m, queue_t *que, int idx);

/*
* recursivly reveal all surrounding squares conected with 0
returns rrue if mine was revealed
*/
bool reveal(mines *m);

/*
* swap hidden to flag and back
*/
void place_flag(mines *m);

/*
*  return true if player already won
*/
bool checkWin(mines *m);

/*
* renders field on LCD
*/
void drawField(mines *m, unsigned short *fb);

/*
* prints score on LCD
*/
void showScore(mines *m, unsigned short *fb);


#endif /*MINES_H*/