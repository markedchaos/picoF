#include "pico/stdlib.h"
#include "ssd1306/ssd1306.h"
#include "gfx/gfx.h"
#include "input/input.h"
#include "registry/registry.h"
#include "hardware_init.h"

//ssd1306_t display;

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
//    ssd1306_init(&display, i2c1, 0x3c, 128, 64);
    
    // --- TEST DRAW: prove the panel works before gfx ---
//    ssd1306_clear(&display);
//    ssd1306_draw_string(&display, 0, 0, "BOOT", 1); // 1 = ON pixels
//    ssd1306_show(&display);
//    sleep_ms(1000); // leave it up for a second
    // ---------------------------------------------------
    hardware_init();
    gfx_init(&disp);
    input_init();
    draw_menu();

    while (true) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    input_update(now);

    if (action_pressed(ACTION_MENU_UP)) {
        selected = (selected - 1 + registry_count()) % registry_count();
        draw_menu();
    }
    if (action_pressed(ACTION_MENU_DOWN)) {
        selected = (selected + 1) % registry_count();
        draw_menu();
    }
    if (action_pressed(ACTION_MENU_SELECT)) {
        registry_entry(selected)->run();
        draw_menu();
    }
}

}




