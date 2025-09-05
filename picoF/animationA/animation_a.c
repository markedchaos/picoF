// animation_a.c
// Animation A: bitwise plasma for 1bpp SSD1306 (128x64 default)
// - No assets in flash
// - ~1 KiB framebuffer in RAM
// - Pure integer math (no floats, no LUTs)
// - Zero heap allocation

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "input/input.h" // added for exit_combo_triggered()

#ifndef AA_DISPLAY_WIDTH
#define AA_DISPLAY_WIDTH 128
#endif
#ifndef AA_DISPLAY_HEIGHT
#define AA_DISPLAY_HEIGHT 64
#endif

#ifndef REGISTER_PROGRAM
#define REGISTER_PROGRAM(fn, name, icon)
#endif

// Presents the 1bpp framebuffer to the OLED.
extern void oled_present_mono_1bpp(const uint8_t* fb, int width, int height);

// ---- Local framebuffer ------------------------------------------------------
static uint8_t s_fb[AA_DISPLAY_WIDTH * (AA_DISPLAY_HEIGHT / 8)]; // 128*64/8 = 1024 bytes

static inline void fb_clear(void) { memset(s_fb, 0, sizeof(s_fb)); }
static inline void fb_set(int x, int y) {
    if ((unsigned)x >= AA_DISPLAY_WIDTH || (unsigned)y >= AA_DISPLAY_HEIGHT) return;
    int index = x + (y >> 3) * AA_DISPLAY_WIDTH;
    uint8_t bit = (uint8_t)(1u << (y & 7));
    s_fb[index] |= bit;
}

// ---- Core animation ---------------------------------------------------------
static inline int iabs_int(int v) { return (v ^ (v >> 31)) - (v >> 31); }

static void render_frame(uint8_t t) {
    fb_clear();
    const int cx = AA_DISPLAY_WIDTH / 2;
    const int cy = AA_DISPLAY_HEIGHT / 2;
    for (int y = 0; y < AA_DISPLAY_HEIGHT; ++y) {
        const int yTerm = (y << 2) + (int)t * 3;
        const int dy = iabs_int(y - cy);
        for (int x = 0; x < AA_DISPLAY_WIDTH; ++x) {
            const int xTerm = (x << 2) + (int)t;
            const int dx = iabs_int(x - cx);
            const uint8_t a = (uint8_t)(xTerm ^ yTerm);
            const uint8_t r = (uint8_t)(dx + dy + ((int)t << 1));
            const uint8_t u = (uint8_t)(a + (r * 5));
            const uint8_t threshold = (uint8_t)(48 + ((t & 0x1F)));
            const bool on = ((uint8_t)(u ^ (t << 2)) < threshold);
            if (on) fb_set(x, y);
        }
    }
}

// Public entry point for the launcher.
void run_animation_a(void) {
    uint8_t t = 0;
    fb_clear();
    oled_present_mono_1bpp(s_fb, AA_DISPLAY_WIDTH, AA_DISPLAY_HEIGHT);

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        input_update(now);
        if (exit_combo_triggered()) {
            fb_clear();
            oled_present_mono_1bpp(s_fb, AA_DISPLAY_WIDTH, AA_DISPLAY_HEIGHT);
            return;
        }

        render_frame(t);
        oled_present_mono_1bpp(s_fb, AA_DISPLAY_WIDTH, AA_DISPLAY_HEIGHT);
        t += 1;
    }
}

REGISTER_PROGRAM(animation_a, "Animation A", NULL);
