#include "hardware.h"

void set_rgb1(int val, unsigned char *mem_base)
{
    int *rgb1 = (int *)(mem_base + SPILED_REG_LED_RGB1_o);
    *rgb1 = val;
}

void set_rgb2(int val, unsigned char *mem_base)
{
    int *rgb2 = (int *)(mem_base + SPILED_REG_LED_RGB2_o);
    *rgb2 = val;
}

void set_leds(int val, unsigned char *mem_base)
{
    int *leds = (int *)(mem_base + SPILED_REG_LED_LINE_o);
    *leds = val;
}

int pready()
{
    char *state = (char *)(SERIAL_PORT_BASE + SERP_TX_ST_REG_o);
    return (*state & SERP_TX_ST_REG_READY_m);
}

void pchar(char c)
{
    char *data = (char *)(SERIAL_PORT_BASE + SERP_TX_DATA_REG_o);
    while (!pready())
        ;
    *data = c;
}

int gready()
{
    char *state = (char *)(SERIAL_PORT_BASE + SERP_RX_ST_REG_o);
    return (*state & SERP_RX_ST_REG_READY_m);
}

char gchar(void)
{
    char *data = (char *)(SERIAL_PORT_BASE + SERP_RX_DATA_REG_o);
    while (!gready())
        ;
    return *data;
}

unsigned int reverseBits(unsigned int num)
{
    unsigned int NO_OF_BITS = sizeof(num) * 8;
    unsigned int reverse_num = 0;
    int i;
    for (i = 0; i < NO_OF_BITS; i++)
    {
        if ((num & (1 << i)))
            reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return reverse_num;
}