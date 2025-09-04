#include <string.h>

#include "gfx.h"



static ssd1306_t* G = NULL;



void gfx_init(ssd1306_t* disp) { G = disp; }

void gfx_clear(void) { if (G) ssd1306_clear(G); }

void gfx_show(void) { if (G) ssd1306_show(G); }



void gfx_plot(int x, int y, bool on) {

    if (!G) return;

    ssd1306_pixel(G, x, y, on);

}



void gfx_hline(int x, int y, int w, bool on) {

    for (int i = 0; i < w; i++) gfx_plot(x + i, y, on);

}

void gfx_fill_rect(int x, int y, int w, int h, bool on) {
    for (int yy = 0; yy < h; yy++) {
        for (int xx = 0; xx < w; xx++) {
            gfx_plot(x + xx, y + yy, on);
        }
    }
}


void gfx_sprite_rows(int x, int y, int w, int h, const char* rows[]) {

    for (int r = 0; r < h; r++) {

        const char* line = rows[r];

        for (int c = 0; c < w; c++) {

            char ch = line[c];

            if (ch == '#' || ch == '1' || ch == 'X')

                gfx_plot(x + c, y + r, true);

        }

    }

}



// 5x7 uppercase alphabet + digits + space. Each byte is a column (LSB=top)

static const uint8_t F_AZ_5x7[26][5] = {

    {0x7E,0x11,0x11,0x7E,0x00}, // A

    {0x7F,0x49,0x49,0x36,0x00}, // B

    {0x3E,0x41,0x41,0x22,0x00}, // C

    {0x7F,0x41,0x41,0x3E,0x00}, // D

    {0x7F,0x49,0x49,0x41,0x00}, // E

    {0x7F,0x09,0x09,0x01,0x00}, // F

    {0x3E,0x41,0x51,0x32,0x00}, // G

    {0x7F,0x08,0x08,0x7F,0x00}, // H

    {0x41,0x7F,0x41,0x00,0x00}, // I

    {0x20,0x40,0x41,0x3F,0x00}, // J

    {0x7F,0x08,0x14,0x63,0x00}, // K

    {0x7F,0x40,0x40,0x40,0x00}, // L

    {0x7F,0x02,0x04,0x02,0x7F}, // M

    {0x7F,0x04,0x08,0x7F,0x00}, // N

    {0x3E,0x41,0x41,0x3E,0x00}, // O

    {0x7F,0x09,0x09,0x06,0x00}, // P

    {0x3E,0x41,0x61,0x3E,0x00}, // Q

    {0x7F,0x09,0x19,0x66,0x00}, // R

    {0x26,0x49,0x49,0x32,0x00}, // S

    {0x01,0x7F,0x01,0x01,0x00}, // T

    {0x3F,0x40,0x40,0x3F,0x00}, // U

    {0x1F,0x20,0x40,0x20,0x1F}, // V

    {0x7F,0x20,0x18,0x20,0x7F}, // W

    {0x63,0x14,0x08,0x14,0x63}, // X

    {0x07,0x08,0x70,0x08,0x07}, // Y

    {0x61,0x51,0x49,0x45,0x43}, // Z

};



static const uint8_t F_09_5x7[10][5] = {

    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0

    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1

    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2

    {0x22, 0x41, 0x49, 0x49, 0x36}, // 3

    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4

    {0x2F, 0x49, 0x49, 0x49, 0x31}, // 5

    {0x3E, 0x49, 0x49, 0x49, 0x30}, // 6

    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7

    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8

    {0x06, 0x49, 0x49, 0x49, 0x3E}  // 9

};



static const uint8_t F_SPACE_5x7[5] = {0,0,0,0,0};



void gfx_char5x7(int x, int y, char ch, bool on) {

    const uint8_t* glyph = F_SPACE_5x7;

    if (ch >= 'A' && ch <= 'Z') glyph = F_AZ_5x7[ch - 'A'];

    else if (ch >= 'a' && ch <= 'z') glyph = F_AZ_5x7[ch - 'a']; // map lowercase to uppercase

    else if (ch >= '0' && ch <= '9') glyph = F_09_5x7[ch - '0'];



    for (int col = 0; col < 5; col++) {

        uint8_t bits = glyph[col];

        for (int row = 0; row < 7; row++) {

            if (bits & (1u << row)) gfx_plot(x + col, y + row, on);

        }

    }

}



void gfx_text5x7(int x, int y, const char* str, bool on) {

    int cx = x;

    while (*str) {

        if (*str == ' ') { cx += 6; str++; continue; }

        gfx_char5x7(cx, y, *str, on);

        cx += 6; // 5px + 1px space

        str++;

    }

}
#include "ssd1306/ssd1306.h"

// Use the display instance from main.c
extern ssd1306_t display;

void oled_present_mono_1bpp(const uint8_t *buf) {
    if (!buf) return;

    // Fast path: exact 128x64 copy
    if (display.width == 128 && display.height == 64) {
        memcpy(display.buf, buf, 1024);
        ssd1306_show(&disp);
        return;
    }

    // TODO: clipped blit path if needed for other sizes
}



