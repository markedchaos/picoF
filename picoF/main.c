#include "pico/stdlib.h"
#include "ssd1306/ssd1306.h"
#include "gfx/gfx.h"
#include "input/input.h"
#include "registry/registry.h"

ssd1306_t display;

static int selected = 0;

static void draw_menu(void) {
    gfx_clear();

    for (int i = 0; i < registry_count(); i++) {
        int y = i * 9; // 8px font + 1px spacing
        if (i == selected) {
            // Draw highlight bar
            gfx_fill_rect(0, y, 128, 8, true); // ON pixels background
            gfx_text5x7(0, y, registry_entry(i)->name, false); // OFF pixels text
        } else {
            gfx_text5x7(0, y, registry_entry(i)->name, true); // ON pixels text
        }
    }

    gfx_show();
}

int main(void) {
    stdio_init_all();
    ssd1306_init(&display, i2c1, 0x3c, 128, 64);
    
    // --- TEST DRAW: prove the panel works before gfx ---
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 0, 0, "BOOT", 1); // 1 = ON pixels
    ssd1306_show(&display);
    sleep_ms(1000); // leave it up for a second
    // ---------------------------------------------------
    
    gfx_init(&display);
    input_init();

    draw_menu();

    while (true) {
        input_update();

        if (input_pressed(BTN_LEFT)) {
            selected = (selected - 1 + registry_count()) % registry_count();
            draw_menu();
        }
        if (input_pressed(BTN_RIGHT)) {
            selected = (selected + 1) % registry_count();
            draw_menu();
        }
        if (input_pressed(BTN_A)) {
            registry_entry(selected)->run();
            draw_menu();
        }
    }
}
