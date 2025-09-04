// animation_b.c
// Animation B: Frame-based pixel art player for SSD1306
// - Consumes frames[] from frames.c (generated from PBM files)
// - Exits cleanly on universal exit combo
// - Includes both FPS-locked and time-based playback (time-based commented out)

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "animationB/frames0.h"       // Generated: provides frames0[], FRAME_COUNT, FRAME_WIDTH, FRAME_HEIGHT
#include "registry.h"     // For REGISTER_PROGRAM
#include "pico/stdlib.h"  // For sleep_ms, absolute_time, etc.

// External hooks from your platform
extern void oled_present_mono_1bpp(const uint8_t* fb, int width, int height);
extern bool input_exit_combo_active(void);

// Local framebuffer (matches display size)
static uint8_t s_fb[FRAME_WIDTH * (FRAME_HEIGHT / 8)];

static inline void fb_clear(void) {
    memset(s_fb, 0, sizeof(s_fb));
}

static inline void fb_blit_frame(const uint8_t* src) {
    memcpy(s_fb, src, sizeof(s_fb));
}

void run_animation_b(void) {
    fb_clear();
    oled_present_mono_1bpp(s_fb, FRAME_WIDTH, FRAME_HEIGHT);

    int frame = 0;

    // ===== FPS-LOCKED PLAYBACK =====
    const int FRAME_DELAY_MS = 100; // 10 FPS
    while (true) {
        if (input_exit_combo_active()) {
            fb_clear();
            oled_present_mono_1bpp(s_fb, FRAME_WIDTH, FRAME_HEIGHT);
            return;
        }

        fb_blit_frame(frames0[frame]);
        oled_present_mono_1bpp(s_fb, FRAME_WIDTH, FRAME_HEIGHT);

        frame = (frame + 1) % FRAME_COUNT;
        sleep_ms(FRAME_DELAY_MS);
    }

    /*
    // ===== TIME-BASED PLAYBACK =====
    // Uncomment this block and comment out the FPS-locked block above to use it.
    const int TARGET_FPS = 10;
    const int FRAME_INTERVAL_MS = 1000 / TARGET_FPS;
    absolute_time_t last_time = get_absolute_time();

    while (true) {
        if (input_exit_combo_active()) {
            fb_clear();
            oled_present_mono_1bpp(s_fb, FRAME_WIDTH, FRAME_HEIGHT);
            return;
        }

        absolute_time_t now = get_absolute_time();
        int elapsed_ms = to_ms_since_boot(now) - to_ms_since_boot(last_time);

        if (elapsed_ms >= FRAME_INTERVAL_MS) {
            last_time = now;
            fb_blit_frame(frames0[frame]);
            oled_present_mono_1bpp(s_fb, FRAME_WIDTH, FRAME_HEIGHT);
            frame = (frame + 1) % FRAME_COUNT;
        }
        // Optional: small sleep to avoid busy-waiting
        sleep_ms(1);
    }
    */
}

REGISTER_PROGRAM(animation_b, "Animation B", NULL);
