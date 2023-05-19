/*******************************************************************
  Project main function template for MicroZed based MZ_APO board
  designed by Petr Porazil at PiKRON

  include your name there and license for distribution.

  Remove next text: This line should not appear in submitted
  work and project name should be change to match real application.
  If this text is there I want 10 points subtracted from final
  evaluation.

 *******************************************************************/

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
//#include "font_rom8x16.c"
//#include "font_prop14x16.c"
#include "font_types.h"
#include "serialize_lock.h"
#include "queue.h"

unsigned short *fb;

typedef struct
{
    int *data; //0 - 8 bomb count, -1 bomb, -2 flag
    int *revealed; //0 hidden, 1 revealed, 2 flag
    int selectedX;
    int selectedY;
    int score;
} mines;

typedef struct
{
    int red_change;
    int green_change;
    int blue_change;
    bool red_pressed;
    bool green_pressed;
    bool blue_pressed;
} rotation_t;

typedef struct
{
  uint8_t red_value;
  uint8_t green_value;
  uint8_t blue_value;
}knob_value;

void generate(mines *m){
    /*randomly generates playfield*/
    //generate bombs
    int bombCount = 0;
    srand(time(NULL));
    while(bombCount < 10){
        int r = rand()%100;
        if(m->data[r] != -1){
            m->data[r] = -1;
            bombCount++;
        }
    } 
    //generate numbers
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            if (m->data[i*10+j] != -1){
                int count = 0;
                if( i != 0){
                    if (m->data[(i-1)*10+j] == -1){
                        count += 1;
                }}
                if( i != 9){
                    if (m->data[(i+1)*10+j] == -1){
                        count += 1;
                }}
                if (j != 0){
                    if (m->data[(i)*10+j-1] == -1){
                        count += 1;
                }
                }
                if (j != 9){
                    if (m->data[(i)*10+j+1] == -1){
                        count += 1;
                    }
                }
                if (i != 0 && j != 0){
                    if (m->data[(i-1)*10+j-1] == -1){
                        count += 1;
                    }
                }
                if (i != 0 && j != 9){
                    if (m->data[(i-1)*10+j+1] == -1){
                        count += 1;
                    }
                }
                if (i != 9 && j != 0){
                    if (m->data[(i+1)*10+j-1] == -1){
                        count += 1;
                    }
                }
                if (i != 9 && j != 9){
                    if (m->data[(i+1)*10+j+1] == -1){
                        count += 1;
                    }
                }
                m->data[i*10+j] = count;
            }
        }
    }
}

void reveal_part(mines *m, queue_t *que, int idx){
    if (m->data[idx] == 0 && m->revealed[idx] == 0){
        if (!contains(que, idx)){
            push_to_queue(que, idx);
        }
    }
    if(m->revealed[idx] == 0){ //
        m->revealed[idx] = 1;
    }
}

void reveal(mines *m){
    m->revealed[m->selectedY*10+m->selectedX] = 1;
    if(m->data[m->selectedY*10+m->selectedX] == 0){
        queue_t* que = create_queue(100);
        push_to_queue(que, m->selectedY*10+m->selectedX);
        while(get_queue_size(que) > 0){
            int idx = get_from_queue(que, 0);
            //if idx != -1
            int i = idx%10;
            int j = idx/10;
            if (i != 0){
                reveal_part(m, que, (j)*10+i-1);
            }
            if (i != 9){
                reveal_part(m, que, (j)*10+i+1);
            }
            if (j != 0){
                reveal_part(m, que, (j-1)*10+i);
            }
            if (j != 9){
                reveal_part(m, que, (j+1)*10+i);
            }
            if (i != 0 && j != 0){
                reveal_part(m, que, (j-1)*10+i-1);
            }
            if (i != 0 && j != 9){
                reveal_part(m, que, (j+1)*10+i-1);
            }
            if (i != 9 && j != 0){
               reveal_part(m, que, (j-1)*10+i+1);
            }
            if(i != 9 && j != 9){
                reveal_part(m, que, (j+1)*10+i+1);
            }
            pop_from_queue(que);
        }
    }
}

void place_flag(mines *m){
    m->revealed[m->selectedY*10+m->selectedX] = 2;
}

void draw_pixel(int x, int y, unsigned short color)
{
    if (x >= 0 && x < 480 && y >= 0 && y < 320)
    {
        fb[x + 480 * y] = color;
    }
}

void print_char(char c, int x, int y, int scale, unsigned short color)
{
    /*print char in better font*/
    /*todo: use prop (proporcional) font*/
    if(c < font_rom8x16.firstchar || c >= font_rom8x16.size - font_rom8x16.firstchar){
        c = font_rom8x16.defaultchar;
    }
    int off = c - font_rom8x16.firstchar;
    const uint16_t *cb = font_rom8x16.bits + (font_rom8x16.height * off);
    for(int i = 0; i < font_rom8x16.height; i++){
        for(int j = 0; j < font_rom8x16.maxwidth; j++){
            if(cb[i] & (0x1 << (15 - j))){
                int px = x + (j * scale);
                int py = y + (i * scale);
                for(int xi = 0; xi < scale; xi++){
                    for(int xj = 0; xj < scale; xj++){
                        draw_pixel(px+xj, py+xi, color);
                        //fb[480*(px + xj)+py + xi] = color;
                    }
                }
            }
        }
    }
}



void draw_square(int startX, int startY, int width, int height, unsigned short color)
{
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            // if(i+startX > 0 && i+startX < 480 && j+startY > 0 && j+startY < 320){
            draw_pixel(i + startX, j + startY, color);
            //}
        }
    }
}

void draw_hollow_square(int startX, int startY, int width, int height, int thick, unsigned short color)
{
    //printf("Called hollow");
    //fflush(stdout);
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            if((i <= thick || i >= width-thick) || (j <= thick || j >= height-thick)){
                draw_pixel(i + startX, j + startY, color);
            }
        }
    }
}

void drawField(mines *m){
    /*drawes squares responsible to playfield*/
    //printf("Called draw");
    //fflush(stdout);
    for (int vert = 0; vert < 10; vert++)
        {
            for (int horz = 0; horz < 10; horz++)
            {
                if (1) //(m->revealed[vert*10+horz] != 0) 
                {
                    if(m->revealed[vert*10+horz] == 1){
                        //shown
                        if(m->data[vert*10+horz] == -1){
                            //mine
                            //printf("drawins %d %d", vert, horz);
                            //fflush(stdout);
                            draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x31);
                        }
                        else{
                            //number
                            //printf("drawins %d %d", vert, horz);
                            //fflush(stdout);
                            draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x7ff);
                            print_char((char)(m->data[vert*10+horz]+48), 90 + (vert * 30) + 10, 20 + (horz * 30) + 10, 1, 0x0ff);
                        }
                        if(m->selectedX == horz && m->selectedY == vert){
                            draw_hollow_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 2, 0x00);
                            //draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x00);
                        }
                    }
                    else{
                        //draw flag
                        draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x70f);
                        print_char((char)('F'), 90 + (vert * 30) + 10, 20 + (horz * 30) + 10, 1, 0x0ff);
                    }
                }
                else
                {
                    //hidden
                    draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x0f0);
                }
            }
        }
}


int get_knobs(unsigned char *mem_base)
{ // int 0-256
    int *knobs = (int *)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    return *knobs;
}

rotation_t get_knob_change(int *lastRotation, unsigned char* mem_base) //-1: left, 0: same, 1: right
{ 
    /*TODO: rotations and buttons*/
    rotation_t result;
    knob_value knob_value;

    uint8_t currentRotation = get_knobs(mem_base);

    uint8_t red_value = ((*lastRotation) >> 16) & 0xff; //Previous position
    uint8_t green_value = ((*lastRotation) >> 8) & 0xff; //Previous position
    uint8_t blue_value = (*lastRotation) & 0xff; //Previous position

    knob_value.red_value = (currentRotation >> 16) & 0xff; //Current position
    knob_value.green_value = (currentRotation >> 8) & 0xff; //Current position
    knob_value.blue_value = (currentRotation) & 0xff; //Current position

    printf("R:%d / %d, G:%d / %d, B:%d / %d\n", red_value, knob_value.red_value, green_value, knob_value.green_value, blue_value, knob_value.blue_value);
    fflush(stdout);

    result.red_change = 0;
    result.green_change = 0;
    result.blue_change = 0;

    //Red knob
    if (knob_value.red_value > red_value)
    {
      if ((knob_value.red_value - red_value) <= 3)
      {
        result.red_change = 1;
        //*lastRotation += (4) >> 16;
      }
      else
      {
        result.red_change = -1;
        //*lastRotation -= (4) >> 16;
      }
    }
    else if (knob_value.red_value < red_value)
    {
      if ((red_value - knob_value.red_value) <= 3)
      {
        result.red_change = -1;
        //*lastRotation -= (4) >> 16;
      }
      else
      {
        result.red_change = 1;
        //*lastRotation += (4) >> 16;
      }
    }
    else
    {
      result.red_change = 0;
    }

    //Green knob
    if (knob_value.green_value > green_value)
    {
      if ((knob_value.green_value - green_value) <= 3)
      {
        result.green_change = 1;
        //*lastRotation += (4) >> 8;
      }
      else
      {
        result.green_change = -1;
        //*lastRotation -= (4) >> 8;
      }
    }
    else if (knob_value.green_value < green_value)
    {
      if ((green_value - knob_value.green_value) <= 3)
      {
        result.green_change = -1;
        //*lastRotation -= (4) >> 8;
      }
      else
      {
        result.green_change = 1;
        //*lastRotation += (4) >> 8;
      }
    }
    else
    {
      result.green_change = 0;
    }

    //Blue knob
    if (knob_value.blue_value > blue_value)
    {
      if ((knob_value.blue_value - blue_value) <= 3)
      {
        result.blue_change = 1;
        //*lastRotation += (4) >> 0;
      }
      else
      {
        result.blue_change = -1;
        //*lastRotation -= (4) >> 0;
      }
    }
    else if (knob_value.blue_value < blue_value)
    {
      if ((blue_value - knob_value.blue_value) <= 3)
      {
        result.blue_change = -1;
        //*lastRotation -= (4) >> 0;
      }
      else
      {
        result.blue_change = 1;
        //*lastRotation += (4) >> 0;
      }
    }
    else
    {
      result.blue_change = 0;
    }
    *lastRotation = currentRotation;

    return result;
}

void set_rgb1(int val)
{
    int *rgb1 = (int *)(SPILED_REG_BASE_PHYS + SPILED_REG_LED_RGB1_o);
    *rgb1 = val;
}

void set_rgb2(int val)
{
    int *rgb2 = (int *)(SPILED_REG_BASE_PHYS + SPILED_REG_LED_RGB2_o);
    *rgb2 = val;
}

int pready()
{ // write ready
    char *state = (char *)(SERIAL_PORT_BASE + SERP_TX_ST_REG_o);
    return (*state & SERP_TX_ST_REG_READY_m);
}

void pchar(char c)
{ // write to console
    char *data = (char *)(SERIAL_PORT_BASE + SERP_TX_DATA_REG_o);
    while (!pready())
        ;
    *data = c;
}

int gready()
{ // read ready
    char *state = (char *)(SERIAL_PORT_BASE + SERP_RX_ST_REG_o);
    return (*state & SERP_RX_ST_REG_READY_m);
}

char gchar(void)
{ // read from console
    char *data = (char *)(SERIAL_PORT_BASE + SERP_RX_DATA_REG_o);
    while (!gready())
        ;
    return *data;
}

union pixel
{
    unsigned short d; // uint16_t d;
    struct
    {
        int b : 5;
        int g : 6;
        int r : 5;
    };
};





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
    m->revealed = malloc(sizeof(bool) * 100);

    for (int i = 0; i < 100; i++)
    {
        m->data[i] = 0;
        m->revealed[i]=0;
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
    // int xx=0, yy=0;
    while (1)
    {

        // test Aretation
        rotation_t change = get_knob_change(&lastRotation, mem_base); // 1, 2, 3 for R G B knobs
        if (change.red_change || change.green_change || change.blue_change ||
        change.red_pressed || change.green_pressed || change.blue_pressed)
        {
            //TODO: on change, change selection in mines, on red click reveal, on green click place flag
            printf("Change: %d %d %d", change.red_change, change.blue_change, change.green_change);
            fflush(stdout);

            m->selectedX = (m->selectedX + change.red_change) % 10;
            m->selectedY = (m->selectedY + change.green_change) % 10;
            if(m->selectedX < 0){
                m->selectedX = 0; //+10 for overlap
            }
            if(m->selectedY < 0){
                m->selectedY = 0; //+10 for overlap
            }
            if(change.red_pressed){
                reveal(m);
            }
            if(change.green_pressed){
                place_flag(m);
            }
        }

        for (ptr = 0; ptr < 320 * 480; ptr++)
        {
            fb[ptr] = 0u;
        }
        // draw square on x, y

        drawField(m);
        //printf("Drawed field\n");
        //fflush(stdout);

        // draw buffer to screen
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
