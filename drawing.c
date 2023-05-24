#include "drawing.h"

void draw_pixel(int x, int y, unsigned short color, unsigned short *fb)
{
    if (x >= 0 && x < 480 && y >= 0 && y < 320)
    {
        fb[x + 480 * y] = color;
    }
}

void draw_square(int startX, int startY, int width, int height, unsigned short color, unsigned short *fb)
{
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            draw_pixel(i + startX, j + startY, color, fb);
        }
    }
}

void draw_hollow_square(int startX, int startY, int width, int height, int thick, unsigned short color, unsigned short *fb)
{
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            if ((i <= thick || i >= width - thick) || (j <= thick || j >= height - thick))
            {
                draw_pixel(i + startX, j + startY, color, fb);
            }
        }
    }
}

void print_char(char c, int x, int y, int scale, unsigned short color, unsigned short *fb)
{
    if (c < font_rom8x16.firstchar || c >= font_rom8x16.size - font_rom8x16.firstchar)
    {
        c = font_rom8x16.defaultchar;
    }
    int off = c - font_rom8x16.firstchar;
    const uint16_t *cb = font_rom8x16.bits + (font_rom8x16.height * off);
    for (int i = 0; i < font_rom8x16.height; i++)
    {
        for (int j = 0; j < font_rom8x16.maxwidth; j++)
        {
            if (cb[i] & (0x1 << (15 - j)))
            {
                int px = x + (j * scale);
                int py = y + (i * scale);
                for (int xi = 0; xi < scale; xi++)
                {
                    for (int xj = 0; xj < scale; xj++)
                    {
                        draw_pixel(px + xj, py + xi, color, fb);
                    }
                }
            }
        }
    }
}

void print_string(char text[], int len, int x, int y, int scale, unsigned short color, unsigned short *fb)
{
    for (int let = 0; let < len; let++)
    {
        char c = text[let];
        if (c < font_winFreeSystem14x16.firstchar || c >= font_winFreeSystem14x16.size - font_winFreeSystem14x16.firstchar)
        {
            c = font_winFreeSystem14x16.defaultchar;
        }
        int off = c - font_winFreeSystem14x16.firstchar;
        const uint16_t *cb = font_winFreeSystem14x16.bits + (font_winFreeSystem14x16.height * off);
        for (int i = 0; i < font_winFreeSystem14x16.height; i++)
        {
            for (int j = 0; j < (int)(font_winFreeSystem14x16.width[off]); j++)
            {
                if (cb[i] & (0x1 << (15 - j)))
                {
                    int px = x + (j * scale);
                    int py = y + (i * scale);
                    for (int xi = 0; xi < scale; xi++)
                    {
                        for (int xj = 0; xj < scale; xj++)
                        {
                            draw_pixel(px + xj, py + xi, color, fb);
                        }
                    }
                }
            }
        }
        x += scale*(int)(font_winFreeSystem14x16.width[off]);
    }
}