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
 
unsigned short *fb;

int get_knobs(void){ //returns 24bits of information (3* 8 bits) = (3* value [0-256]) 
    int * knobs = (int*)(SPILED_REG_BASE_PHYS + SPILED_REG_KNOBS_8BIT_o);
    return *knobs;
}

int get_knob_change(int *lastRotation){ //-1: left, 0: same, 1: right
  //remake this function, use mask to get specific knob, then process its rotation change relative to its last rotation
    int currentRotation = get_knobs();
    if(currentRotation - (*lastRotation) > 3){
        (*lastRotation) += 4;
        return 1;
    }
    else if(currentRotation - (*lastRotation) > -3 &&currentRotation - (*lastRotation) < -200){ //testValue
        (*lastRotation) -= 4;
        return -1;
    }
    else{
        return 0;
    }
}

void set_rgb1(int val){
    int * rgb1 = (int*)(SPILED_REG_BASE_PHYS + SPILED_REG_LED_RGB1_o);
    *rgb1 = val;
}

void set_rgb2(int val){
    int * rgb2 = (int*)(SPILED_REG_BASE_PHYS + SPILED_REG_LED_RGB2_o);
    *rgb2 = val;
}

int pready(){ //write ready
    char *state = (char*)(SERIAL_PORT_BASE + SERP_TX_ST_REG_o);
    return (*state & SERP_TX_ST_REG_READY_m);
}

void pchar(char c){ //write to console
    char *data = (char*)(SERIAL_PORT_BASE + SERP_TX_DATA_REG_o);
    while(!pready());
    *data = c;
}

int gready(){ //read ready
    char *state = (char*)(SERIAL_PORT_BASE + SERP_RX_ST_REG_o);
    return (*state & SERP_RX_ST_REG_READY_m);
}

char gchar(void){ //read from console
    char *data = (char*)(SERIAL_PORT_BASE + SERP_RX_DATA_REG_o);
    while(!gready());
    return *data;
}

union pixel{
    unsigned short d; //uint16_t d;
    struct
    {
        int b : 5;
        int g : 6;
        int r : 5;
    };
};




void draw_pixel(int x, int y, unsigned short color) {
  if (x>=0 && x<480 && y>=0 && y<320) {
    fb[x+480*y] = color;
  }
}
 
 
int main(int argc, char *argv[]) {
  unsigned char *parlcd_mem_base, *mem_base;
  int i,j;
  int ptr;
  unsigned int c;
  int lastRotation = get_knobs();
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
  for (i = 0; i < 320 ; i++) {
    for (j = 0; j < 480 ; j++) {
      c = 0;
      fb[ptr]=c;
      parlcd_write_data(parlcd_mem_base, fb[ptr++]);
    }
  }
 
  struct timespec loop_delay;
  loop_delay.tv_sec = 0;
  loop_delay.tv_nsec = 150 * 1000 * 1000;
  int xx=0, yy=0;
  while (1) {

    //test Aretation
    int change = get_knob_change(&lastRotation);
    if(change != 0){
        printf("Change: %d", change);
    }







    //get knobs data
    int r = *(volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    if ((r&0x7000000)!=0) {
      break;
    }
    //some magic to make them xx and yy
    xx = ((r&0xff)*480)/256;
    yy = (((r>>8)&0xff)*320)/256;
    //clear screen
    for (ptr = 0; ptr < 320*480 ; ptr++) {
        fb[ptr]=0u;
    }
    //draw square on xx, yy
    for (j=0; j<60; j++) {
      for (i=0; i<60; i++) {
        draw_pixel(i+xx,j+yy,0x7ff);
      }
    }
    //draw buffer to screen
    parlcd_write_cmd(parlcd_mem_base, 0x2c);
    for (ptr = 0; ptr < 480*320 ; ptr++) {
        parlcd_write_data(parlcd_mem_base, fb[ptr]);
    }
    //wait for another loop
    clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
  }
 
 //clear screen
  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  for (ptr = 0; ptr < 480*320 ; ptr++) {
    parlcd_write_data(parlcd_mem_base, 0);
  }
 
  printf("Goodbye world\n");
 
  return 0;
}
