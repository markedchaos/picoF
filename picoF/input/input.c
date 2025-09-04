#include "input.h"
#include "pico/stdlib.h"

#define BTN_GPIO_UP     2
#define BTN_GPIO_DOWN   3
#define BTN_GPIO_LEFT   4
#define BTN_GPIO_RIGHT  5
#define BTN_GPIO_A      6
#define BTN_GPIO_B      7

#define EXIT_HOLD_MS    2000
#define FAIL_HOLD_MS    5000
#define DEBOUNCE_MS     30

typedef struct {
    uint8_t gpio;
    bool current;
    bool previous;
    absolute_time_t last_change;
    absolute_time_t press_time;
} ButtonState;

static ButtonState buttons[BTN_COUNT];

static void set_button(ButtonId id, uint8_t gpio) {
    buttons[id].gpio = gpio;
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
    buttons[id].current = buttons[id].previous = false;
    buttons[id].last_change = get_absolute_time();
    buttons[id].press_time = 0;
}

void input_init(void) {
    set_button(BTN_UP, BTN_GPIO_UP);
    set_button(BTN_DOWN, BTN_GPIO_DOWN);
    set_button(BTN_LEFT, BTN_GPIO_LEFT);
    set_button(BTN_RIGHT, BTN_GPIO_RIGHT);
    set_button(BTN_A, BTN_GPIO_A);
    set_button(BTN_B, BTN_GPIO_B);
}

void input_update(void) {
    for (int i = 0; i < BTN_COUNT; i++) {
        bool raw = !gpio_get(buttons[i].gpio); // active low
        absolute_time_t now = get_absolute_time();
        if (raw != buttons[i].current &&
            absolute_time_diff_us(buttons[i].last_change, now) / 1000 > DEBOUNCE_MS) {
            buttons[i].previous = buttons[i].current;
            buttons[i].current = raw;
            buttons[i].last_change = now;
            if (raw) {
                buttons[i].press_time = now;
            }
        }
    }
}

bool input_pressed(ButtonId btn) {
    return (buttons[btn].current && !buttons[btn].previous);
}

bool input_held(ButtonId btn) {
    return buttons[btn].current;
}

bool input_released(ButtonId btn) {
    return (!buttons[btn].current && buttons[btn].previous);
}

bool input_exit_requested(void) {
    absolute_time_t now = get_absolute_time();
    return (buttons[BTN_B].current &&
            absolute_time_diff_us(buttons[BTN_B].press_time, now) / 1000 >= EXIT_HOLD_MS);
}

bool input_fail_safe(void) {
    absolute_time_t now = get_absolute_time();
    return (buttons[BTN_B].current &&
            absolute_time_diff_us(buttons[BTN_B].press_time, now) / 1000 >= FAIL_HOLD_MS);
}

bool input_exit_combo_active(void) {
    return input_exit_requested() || input_fail_safe()
           || (input_pressed(BTN_A) && input_pressed(BTN_B));
}
