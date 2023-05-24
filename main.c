
#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_types.h"
#include "serialize_lock.h"
#include "queue.h"
#include "knobs.h"
#include "mines.h"
#include "hardware.h"
#include "drawing.h"

unsigned short *fb;



int main(int argc, char *argv[])
{
    if (serialize_lock(1) <= 0)
    {
        printf("System is occupied\n");

        if (1)
        {
            printf("Waitting\n");
            /* Wait till application holding lock releases it or exits */
            serialize_lock(0);
        }
    }

    mines *m = malloc(sizeof(mines));
    m->data = malloc(sizeof(int) * 100);
    m->revealed = malloc(sizeof(int) * 100);

    for (int i = 0; i < 100; i++)
    {
        m->data[i] = 0;
        m->revealed[i] = 0;
    }
    m->score = 0;
    m->selectedX = 0;
    m->selectedY = 0;

    generate(m);

    unsigned char *parlcd_mem_base, *mem_base;
    int i, j;
    int ptr;
    unsigned int c;

    fb = (unsigned short *)malloc(320 * 480 * 2);

    printf("Hello world\n");
    fflush(stdout);

    parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    if (parlcd_mem_base == NULL)
        exit(1);

    mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    if (mem_base == NULL)
        exit(1);

    parlcd_hx8357_init(parlcd_mem_base);

    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    ptr = 0;
    // init screen buffer
    for (i = 0; i < 320; i++)
    {
        for (j = 0; j < 480; j++)
        {
            c = 0;
            fb[ptr] = c;
            parlcd_write_data(parlcd_mem_base, fb[ptr++]);
        }
    }

    int lastRotation = get_knobs(mem_base);

    struct timespec loop_delay;
    loop_delay.tv_sec = 0;
    loop_delay.tv_nsec = 150 * 1000 * 1000;
    
    int gameStage = 0;
    set_rgb1(50, mem_base);
    set_rgb2(50, mem_base);

    while (1)
    {
        if (gameStage == 0)
        {   //game progress
            rotation_t change = get_knob_change(&lastRotation, mem_base);
            if (change.red_change || change.green_change || change.blue_change || change.red_click || change.green_click || change.blue_click)
            {

                m->selectedX = (m->selectedX + change.green_change);
                m->selectedY = (m->selectedY + change.red_change);
                if (m->selectedX > 9)
                {
                    m->selectedX = 9; //-10 for overlap
                }
                if (m->selectedX < 0)
                {
                    m->selectedX = 0; //+10 for overlap
                }
                if (m->selectedY > 9)
                {
                    m->selectedY = 9; //-10 for overlap
                }
                if (m->selectedY < 0)
                {
                    m->selectedY = 0; //+10 for overlap
                }
                if (change.red_click)
                {
                    if (reveal(m))
                    {
                        gameStage = 2;
                    };
                }
                if (change.green_click)
                {
                    place_flag(m);
                }
            }

            for (ptr = 0; ptr < 320 * 480; ptr++)
            {
                fb[ptr] = 0u;
            }

            drawField(m, fb);
            showScore(m, fb);
            float level = (m->score / 100.0) * 32;
            int reg = 0;
            reg = ~(reg & 0);
            reg = ~(reg << (int)level);
            reg = reverseBits(reg);
            set_leds(reg, mem_base);
            if (checkWin(m))
            {
                gameStage = 1;
            }
        }
        else if (gameStage == 1)
        { // won - menu to start again
            for (ptr = 0; ptr < 320 * 480; ptr++)
            {
                fb[ptr] = 0u;
            }
            print_string("You Won", 7, 100, 50, 4, 0x7ff, fb);
            print_string("Play Again", 10, 100, 100, 4, 0x1f<<11, fb);
            print_string("Exit", 4, 180, 200, 4, 0x32, fb);
            set_rgb1(2016, mem_base);
            set_rgb2(2016, mem_base);
            rotation_t change = get_knob_change(&lastRotation, mem_base); // 1, 2, 3 for R G B knobs
            if (change.red_change || change.green_change || change.blue_change || change.red_click || change.green_click || change.blue_click)
            {
                if (change.red_click)
                {
                    m->score = 0;
                    m->selectedX = 0;
                    m->selectedY = 0;
                    generate(m);

                    gameStage = 1;
                }
                if (change.blue_click)
                {
                    gameStage = 3;
                }
            }
        }
        else if (gameStage == 2)
        { // lost - menu to start again
            for (ptr = 0; ptr < 320 * 480; ptr++)
            {
                fb[ptr] = 0u;
            }
            print_string("You Lost", 8, 120, 50, 4,0x7ff, fb);
            print_string("Play Again", 10, 100, 100, 4, 0x1f<<11, fb);
            print_string("Exit", 4, 180, 200, 4, 0x32, fb);
            set_rgb1(31<<11, mem_base);
            set_rgb2(31<<11, mem_base);
            rotation_t change = get_knob_change(&lastRotation, mem_base); // 1, 2, 3 for R G B knobs
            if (change.red_change || change.green_change || change.blue_change || change.red_click || change.green_click || change.blue_click)
            {
                if (change.red_click)
                {
                    m->score = 0;
                    m->selectedX = 0;
                    m->selectedY = 0;
                    generate(m);

                    gameStage = 1;
                }
                if (change.blue_click)
                {
                    gameStage = 3;
                }
            }
        }
        if (gameStage == 3)
        {
            break;
        }

        parlcd_write_cmd(parlcd_mem_base, 0x2c);
        for (ptr = 0; ptr < 480 * 320; ptr++)
        {
            parlcd_write_data(parlcd_mem_base, fb[ptr]);
        }
        // wait for another loop
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }

    // clear screen
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < 480 * 320; ptr++)
    {
        parlcd_write_data(parlcd_mem_base, 0);
    }

    printf("Goodbye world\n");
    serialize_unlock();

    return 0;
}
