// animation_a.c
// Animation A: bitwise plasma for 1bpp SSD1306 (128x64 default)
// - No assets in flash
// - ~1 KiB framebuffer in RAM
// - Pure integer math (no floats, no LUTs)
// - Zero heap allocation
//
// Integration contract (define or provide these):
//   1) void oled_present_mono_1bpp(const uint8_t* fb, int width, int height);
//      - Presents a page-major 1bpp buffer to the OLED.
//      - Buffer format: width = AA_DISPLAY_WIDTH, height = AA_DISPLAY_HEIGHT.
//        Layout: pages of 8 rows: index = x + (y>>3) * width; bit = 1 << (y & 7)
//   2) bool input_exit_combo_active(void);
//      - Return true when your universal exit gesture is active.
//   3) Optional: You can throttle frame rate inside oled_present_mono_1bpp or here.
//
// Auto-registration:
//   If your launcher defines REGISTER_PROGRAM(name, fn), this module registers itself.
//   Otherwise the macro is a no-op and you can call run_animation_a() directly.

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#ifndef AA_DISPLAY_WIDTH
#define AA_DISPLAY_WIDTH 128
#endif

#ifndef AA_DISPLAY_HEIGHT
#define AA_DISPLAY_HEIGHT 64
#endif

// If your registry macro exists, it will be used; else no-op.
#ifndef REGISTER_PROGRAM
#define REGISTER_PROGRAM(fn, name, icon)
#endif

// Required integration hooks (you implement these elsewhere).
// Presents the 1bpp framebuffer to the OLED.
extern void oled_present_mono_1bpp(const uint8_t* fb, int width, int height);
// Indicates if the universal exit combo is currently active.
extern bool input_exit_combo_active(void);

// ---- Local framebuffer ------------------------------------------------------

static uint8_t s_fb[AA_DISPLAY_WIDTH * (AA_DISPLAY_HEIGHT / 8)]; // 128*64/8 = 1024 bytes

static inline void fb_clear(void) {
    memset(s_fb, 0, sizeof(s_fb));
}

static inline void fb_set(int x, int y) {
    // Bounds guard (safe and cheap)
    if ((unsigned)x >= AA_DISPLAY_WIDTH || (unsigned)y >= AA_DISPLAY_HEIGHT) return;
    int index = x + (y >> 3) * AA_DISPLAY_WIDTH;
    uint8_t bit = (uint8_t)(1u << (y & 7));
    s_fb[index] |= bit;
}

// ---- Core animation ---------------------------------------------------------

// Cheap absolute value for ints
static inline int iabs_int(int v) { return (v ^ (v >> 31)) - (v >> 31); }

// Render one frame into s_fb using bitwise/radial interference.
// Tunables are chosen for flowy motion without heavy math.
static void render_frame(uint8_t t) {
    fb_clear();

    // Center for radial term
    const int cx = AA_DISPLAY_WIDTH  / 2;
    const int cy = AA_DISPLAY_HEIGHT / 2;

    // Lightweight spatial scalars (bitshifts avoid 32-bit multiplies)
    // x_term = (x << 2) + t
    // y_term = (y << 2) + 3*t
    // r_term = |x-cx| + |y-cy| + 2*t
    for (int y = 0; y < AA_DISPLAY_HEIGHT; ++y) {
        const int yTerm = (y << 2) + (int)t * 3;
        const int dy    = iabs_int(y - cy);
        for (int x = 0; x < AA_DISPLAY_WIDTH; ++x) {
            const int xTerm = (x << 2) + (int)t;
            const int dx    = iabs_int(x - cx);

            // Interference pattern (wrap naturally in 8-bit)
            const uint8_t a = (uint8_t)(xTerm ^ yTerm);
            const uint8_t r = (uint8_t)(dx + dy + ((int)t << 1));
            const uint8_t u = (uint8_t)(a + (r * 5));  // 5 adds a gentle “ring” influence

            // Animated threshold; the xor with a moving bit toggles contours for shimmer
            const uint8_t threshold = (uint8_t)(48 + ((t & 0x1F))); // 48..79
            const bool on = ((uint8_t)(u ^ (t << 2)) < threshold);

            if (on) fb_set(x, y);
        }
    }
}

// Public entry point for the launcher.
void run_animation_a(void) {
    uint8_t t = 0;

    // Simple warm start; ensure clean frame on entry
    fb_clear();
    oled_present_mono_1bpp(s_fb, AA_DISPLAY_WIDTH, AA_DISPLAY_HEIGHT);

    while (true) {
        if (input_exit_combo_active()) {
            // Clear before exit to avoid ghosting if your presenter preserves buffer
            fb_clear();
            oled_present_mono_1bpp(s_fb, AA_DISPLAY_WIDTH, AA_DISPLAY_HEIGHT);
            return;
        }

        render_frame(t);
        oled_present_mono_1bpp(s_fb, AA_DISPLAY_WIDTH, AA_DISPLAY_HEIGHT);

        // Time step: tweak for speed; smaller steps => slower motion, bigger => faster
        t += 1;
        // Optional: if you need a cap, sleep inside your presenter or add a small delay here.
    }
}

// Auto-register with your registry-driven launcher if available.
REGISTER_PROGRAM(animation_a, "Animation A", NULL);
