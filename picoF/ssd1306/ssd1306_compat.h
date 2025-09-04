#pragma once
#include "ssd1306.h"

// You must have a global or file‑scope ssd1306_t instance named `disp`
// that is already initialized before calling these.

#define ssd1306_fill_rect(x, y, w, h, c)   ssd1306_rect(&disp, x, y, w, h, c)
#define ssd1306_draw_string(x, y, str, c, bg) ssd1306_string(&disp, x, y, str, c)
#define ssd1306_draw_pixel(x, y, c)        ssd1306_pixel(&disp, x, y, c)
#define ssd1306_clear()                    ssd1306_clear(&disp)
#define ssd1306_show()                     ssd1306_show(&disp)
