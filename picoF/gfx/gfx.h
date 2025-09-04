#ifndef GFX_H

#define GFX_H



#include <stdbool.h>

#include "ssd1306.h"



#ifdef __cplusplus

extern "C" {

#endif



// Thin wrapper around ssd1306 for simple drawing/text

void gfx_init(ssd1306_t* disp);

void gfx_clear(void);

void gfx_show(void);

void gfx_plot(int x, int y, bool on);

void gfx_hline(int x, int y, int w, bool on);

void gfx_fill_rect(int x, int y, int w, int h, bool on);

void oled_present_mono_1bpp(const uint8_t *buf);



// 5x7 uppercase + digits font rendering (A-Z, 0-9, space)

void gfx_char5x7(int x, int y, char ch, bool on);

void gfx_text5x7(int x, int y, const char* str, bool on);



// Draw monochrome sprite from rows of '.' and '#' (or '1','X')

void gfx_sprite_rows(int x, int y, int w, int h, const char* rows[]);



#ifdef __cplusplus

}

#endif



#endif // GFX_H

