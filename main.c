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
#include <time.h>
#include <unistd.h>
#include <termios.h>            //termios, TCSANOW, ECHO, ICANON

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "serialize_lock.h"
 
unsigned short *fb;

#define RED_KNOB 16
#define GREEN_KNOB 8
#define BLUE_KNOB 0

#define RED_CLICK 26
#define GREEN_CLICK 25
#define BLUE_CLICK 24

typedef struct
{
    int red_change;
    int blue_change;
    int green_change;
}rotation_t;

typedef struct
{
  uint8_t red_old;
  uint8_t green_old;
  uint8_t blue_old;

  uint8_t red_new;
  uint8_t green_new;
  uint8_t blue_new;
}knob_value;


int get_knobs(unsigned char *mem_base) //int 0-256
{  
    volatile uint32_t* knobs = (volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    return *knobs;
}

rotation_t get_knob_change(int *lastRotation, unsigned char* mem_base) //-1: left, 0: same, 1: right
{ 
    rotation_t result;
    knob_value knob_value;

    uint32_t currentRotation = get_knobs(mem_base);

    knob_value.red_new = (*lastRotation >> RED_KNOB) & 0xFF; //Previous position
    knob_value.green_new = (*lastRotation >> GREEN_KNOB) & 0xFF; //Previous position
    knob_value.blue_new = (*lastRotation >> BLUE_KNOB) & 0xFF; //Previous position

    knob_value.red_old = (currentRotation >> RED_KNOB) & 0xFF; //Current position
    knob_value.green_old = (currentRotation >> GREEN_KNOB) & 0xFF; //Current position
    knob_value.blue_old = (currentRotation >> BLUE_KNOB) & 0xFF; //Current position

    result.red_change = 0;
    result.green_change = 0;
    result.blue_change = 0;

    //Red knob
    if (knob_value.red_old > knob_value.red_new)
    {
      if ((knob_value.red_old - knob_value.red_new) <= 128)
      {
        result.red_change = 1;
        *lastRotation += (4 << RED_KNOB);
        //printf("Current: %d | Last: %d\n", knob_value.red_old, knob_value.red_new);
      }
      else
      {
        result.red_change = -1;
        *lastRotation -= (4 << RED_KNOB);
        //printf("Current: %d | Last: %d\n", knob_value.red_old, knob_value.red_new);
      }
    }
    else if (knob_value.red_old < knob_value.red_new)
    {
      if ((knob_value.red_new - knob_value.red_old) <= 128)
      {
        result.red_change = -1;
        *lastRotation -= (4 << RED_KNOB);
        //printf("Current: %d | Last: %d\n", knob_value.red_old, knob_value.red_new);
      }
      else
      {
        result.red_change = 1;
        *lastRotation += (4 << RED_KNOB);
        //printf("Current: %d | Last: %d\n", knob_value.red_old, knob_value.red_new);
      }
    }
    else
    {
      result.red_change = 0;
      //printf("Current: %d | Last: %d\n", knob_value.red_old, knob_value.red_new);
    }

    //Green knob
    if (knob_value.green_old > knob_value.green_new)
    {
      if ((knob_value.green_old - knob_value.green_new) <= 128)
      {
        result.green_change = 1;
        *lastRotation += (4 << GREEN_KNOB);
        printf("Current: %d | Last: %d\n", knob_value.green_old, knob_value.green_new);
      }
      else
      {
        result.green_change = -1;
        *lastRotation -= (4 << GREEN_KNOB);
        printf("Current: %d | Last: %d\n", knob_value.green_old, knob_value.green_new);
      }
    }
    else if (knob_value.green_old < knob_value.green_new)
    {
      if ((knob_value.green_new - knob_value.green_old) <= 128)
      {
        result.green_change = -1;
        *lastRotation -= (4 << GREEN_KNOB);
        printf("Current: %d | Last: %d\n", knob_value.green_old, knob_value.green_new);
      }
      else
      {
        result.green_change = 1;
        *lastRotation += (4 << GREEN_KNOB);
        printf("Current: %d | Last: %d\n", knob_value.green_old, knob_value.green_new);
      }
    }
    else
    {
      result.green_change = 0;
      printf("Current: %d | Last: %d\n", knob_value.green_old, knob_value.green_new);
    }

    //Blue knob
    if (knob_value.blue_old > knob_value.blue_new)
    {
      if ((knob_value.blue_old - knob_value.blue_new) <= 128)
      {
        result.blue_change = 1;
        *lastRotation += (4 << BLUE_KNOB);
      }
      else
      {
        result.blue_change = -1;
        *lastRotation -= (4 << BLUE_KNOB);
      }
    }
    else if (knob_value.blue_old < knob_value.blue_new)
    {
      if ((knob_value.blue_new - knob_value.blue_old) <= 128)
      {
        result.blue_change = -1;
        *lastRotation -= (4 << BLUE_KNOB);
      }
      else
      {
        result.blue_change = 1;
        *lastRotation += (4 << BLUE_KNOB);
      }
    }
    else
    {
      result.blue_change = 0;
    }

    return result;
}

void set_rgb1(int val)
{
    int * rgb1 = (int*)(SPILED_REG_BASE_PHYS + SPILED_REG_LED_RGB1_o);
    *rgb1 = val;
}

void set_rgb2(int val)
{
    int * rgb2 = (int*)(SPILED_REG_BASE_PHYS + SPILED_REG_LED_RGB2_o);
    *rgb2 = val;
}

int pready() //write ready
{ 
    char *state = (char*)(SERIAL_PORT_BASE + SERP_TX_ST_REG_o);
    return (*state & SERP_TX_ST_REG_READY_m);
}

void pchar(char c) //write to console
{ 
    char *data = (char*)(SERIAL_PORT_BASE + SERP_TX_DATA_REG_o);
    while(!pready());
    *data = c;
}

int gready() //read ready
{ 
    char *state = (char*)(SERIAL_PORT_BASE + SERP_RX_ST_REG_o);
    return (*state & SERP_RX_ST_REG_READY_m);
}

char gchar(void) //read from console
{ 
    char *data = (char*)(SERIAL_PORT_BASE + SERP_RX_DATA_REG_o);
    while(!gready());
    return *data;
}

union pixel
{
    unsigned short d; //uint16_t d;
    struct
    {
        int b : 5;
        int g : 6;
        int r : 5;
    };
};




void draw_pixel(int x, int y, unsigned short color) 
{
  if (x>=0 && x<480 && y>=0 && y<320) 
  {
    fb[x+480*y] = color;
  }
}

void draw_square(int startX, int startY, int width, int height, unsigned short color)
{
    for (int j=0; j<height; j++) 
    {
      for (int i=0; i<width; i++) 
      {
        //if(i+startX > 0 && i+startX < 480 && j+startY > 0 && j+startY < 320){
        draw_pixel(i+startX, j+startY, color);
        //}
      }
    }
}


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

  unsigned char *parlcd_mem_base, *mem_base;
  int i,j;
  int ptr;
  unsigned int c;
  
  fb  = (unsigned short *)malloc(320*480*2);
 
  printf("Hello world\n");
 
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  if (parlcd_mem_base == NULL)
    exit(1);
 
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  if (mem_base == NULL)
    exit(1);
 
  parlcd_hx8357_init(parlcd_mem_base);
 
  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  ptr=0;
  //init screen buffer
  for (i = 0; i < 320 ; i++) 
  {
    for (j = 0; j < 480 ; j++) 
    {
      c = 0;
      fb[ptr]=c;
      parlcd_write_data(parlcd_mem_base, fb[ptr++]);
    }
  }

  int lastRotation = get_knobs(mem_base);

  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 150 * 1000 * 1000;
  //int xx=0, yy=0;
  while (1) 
  {
    
    //test Aretation
    rotation_t change = get_knob_change(&lastRotation, mem_base); //1, 2, 3 for R G B knobs
    printf("Change: R: %d | G: %d | B: %d\n", change.red_change, change.green_change, change.blue_change);
    fflush(stdout);


    //get knobs data
    /*int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if ((r&0x7000000)!=0) {
      break;
    }*/
    //some magic to make them xx and yy
    /*xx = ((r&0xff)*480)/256;
    yy = (((r>>8)&0xff)*320)/256;*/
    //clear screen
    for (ptr = 0; ptr < 320*480 ; ptr++) 
    {
        fb[ptr]=0u;
    }
    //draw square on x, y

    for(int vert = 0; vert < 5*20; vert+=20)
    {
        for(int horz = 0; horz < 5*20; horz+=20)
        {
            if((horz+vert) % 40 == 0)
            {
                draw_square(vert, horz, 20, 20, 0x7ff);
                //printf("Wh %d, %d ", vert, horz);
            }
            else
            {
                draw_square(vert, horz, 20, 20, 0x0f0);
                //printf("BL %d, %d ", vert, horz);
            }
        }
    }
    //printf("\n");
    //fflush(stdout);
    
    //draw buffer to screen
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < 480*320 ; ptr++) 
    {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
    //wait for another loop
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
 
 //clear screen
  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  for (ptr = 0; ptr < 480*320 ; ptr++)
  {
    parlcd_write_data(parlcd_mem_base, 0);
  }
 
  printf("Goodbye world\n");
  serialize_unlock();
 
  return 0;
}
