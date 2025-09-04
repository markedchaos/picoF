#ifndef _SSD1306_H
#define _SSD1306_H

#include <stdbool.h>
#include "hardware/i2c.h"

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT   64

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t pages;
    uint8_t address;
    i2c_inst_t *i2c;
    uint8_t buf[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
} ssd1306_t;

// Initialise the display
void ssd1306_init(ssd1306_t* s, i2c_inst_t* i2c, uint8_t addr, uint8_t w, uint8_t h);

// Clear the framebuffer
void ssd1306_clear(ssd1306_t* s);

// Send the framebuffer to the display
void ssd1306_show(ssd1306_t* s);

// Draw a single pixel
void ssd1306_pixel(ssd1306_t* s, int x, int y, bool colour);

// Fill a rectangle
void ssd1306_rect(ssd1306_t* s, int x, int y, int w, int h, bool colour);

// Draw a character (5x7 font)
void ssd1306_char(ssd1306_t* s, int x, int y, char c, bool colour);

// Draw a string
void ssd1306_string(ssd1306_t* s, int x, int y, const char* str, bool colour);

#endif
