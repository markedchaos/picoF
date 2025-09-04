#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

// Logical button IDs
typedef enum {
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_A,
    BTN_B,
    BTN_COUNT
} ButtonId;

// Initialize GPIOs and state tracking
void input_init(void);

// Call once per frame/tick to update button states
void input_update(void);

// Query functions
bool input_pressed(ButtonId btn);   // true on press edge
bool input_held(ButtonId btn);      // true while held
bool input_released(ButtonId btn);  // true on release edge

// Exit gesture checks
bool input_exit_requested(void);    // 2s hold
bool input_fail_safe(void);         // 5s hold
bool input_exit_combo_active(void);


#endif
