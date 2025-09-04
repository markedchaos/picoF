#include "input.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <string.h>

#define BTN_COUNT 3
#define BTN0_PIN 9   // Left
#define BTN1_PIN 8   // Middle
#define BTN2_PIN 7   // Right
#define BTN_ACTIVE_LOW 0   // 0 = active-high
#define BTN_PULL     0     // 0 = pull-down
#define BTN_DEBOUNCE_MS 20
#define BTN_HELD_MS     400

typedef struct {
    bool raw;
    bool debounced;
    bool prev;
    uint32_t last_ms;
    uint32_t since_ms;
} btn_t;

static btn_t btns[BTN_COUNT];

// Mapping table: physical index -> logical action per program
static const Action mapping[PROGRAM_MAX_][BTN_COUNT] = {
    [PROGRAM_MENU]     = { ACTION_MENU_UP,     ACTION_MENU_SELECT, ACTION_MENU_DOWN },
    [PROGRAM_BRICKOUT] = { ACTION_PADDLE_LEFT, ACTION_LAUNCH,      ACTION_PADDLE_RIGHT },
    [PROGRAM_DINO]     = { ACTION_DUCK,        ACTION_RESTART,     ACTION_JUMP },
    [PROGRAM_ANIMATION]= { ACTION_NONE,        ACTION_NONE,        ACTION_NONE },
};

// Stub — replace with your launcher’s getter
ProgramID current_program_id(void) {
    return PROGRAM_MENU;
}

static inline uint btn_pin(int i) {
    switch (i) {
        case 0: return BTN0_PIN;
        case 1: return BTN1_PIN;
        case 2: return BTN2_PIN;
        default: return 0;
    }
}

static inline bool read_active(int i) {
    bool level = gpio_get(btn_pin(i));
#if BTN_ACTIVE_LOW
    return !level;
#else
    return level;
#endif
}

static inline void init_button_pin(uint btn_pin) {
    gpio_init(btn_pin);
    gpio_set_dir(btn_pin, false);
#if BTN_PULL
    gpio_pull_up(btn_pin);
#else
    gpio_pull_down(btn_pin);
#endif
}

void input_init(void) {
    init_button_pin(BTN0_PIN);
    init_button_pin(BTN1_PIN);
    init_button_pin(BTN2_PIN);

    uint32_t now = to_ms_since_boot(get_absolute_time());
    memset(btns, 0, sizeof(btns));
    for (int i = 0; i < BTN_COUNT; i++) {
        btns[i].raw = read_active(i);
        btns[i].debounced = btns[i].raw;
        btns[i].prev = btns[i].debounced;
        btns[i].last_ms = now;
        btns[i].since_ms = now;
    }
}

void input_update(uint32_t now_ms) {
    for (int i = 0; i < BTN_COUNT; i++) {
        bool r = read_active(i);
        if (r != btns[i].raw) {
            btns[i].raw = r;
            btns[i].last_ms = now_ms;
        }
        if (btns[i].debounced != btns[i].raw) {
            if ((now_ms - btns[i].last_ms) >= BTN_DEBOUNCE_MS) {
                btns[i].prev = btns[i].debounced;
                btns[i].debounced = btns[i].raw;
                btns[i].since_ms = now_ms;
            }
        } else {
            btns[i].prev = btns[i].debounced;
        }
    }
}

// Physical queries
bool input_pressed(int idx) {
    return (idx >= 0 && idx < BTN_COUNT) && (btns[idx].debounced && !btns[idx].prev);
}
bool input_released(int idx) {
    return (idx >= 0 && idx < BTN_COUNT) && (!btns[idx].debounced && btns[idx].prev);
}
bool input_held(int idx) {
    if (idx < 0 || idx >= BTN_COUNT) return false;
    if (!btns[idx].debounced) return false;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    return (now - btns[idx].since_ms) >= BTN_HELD_MS;
}

// Logical queries
static bool any_button_for_action(Action a, bool (*pred)(int)) {
    ProgramID p = current_program_id();
    for (int i = 0; i < BTN_COUNT; i++) {
        if (mapping[p][i] == a && pred(i)) return true;
    }
    return false;
}

bool action_pressed(Action a)  { return any_button_for_action(a, input_pressed); }
bool action_released(Action a) { return any_button_for_action(a, input_released); }
bool action_held(Action a)     { return any_button_for_action(a, input_held); }

// Exit combo: hold Left (0) + Right (2)
bool exit_combo_triggered(void) {
    return input_held(0) && input_held(2);
}
