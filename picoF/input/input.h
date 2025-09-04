#pragma once
#include <stdbool.h>
#include <stdint.h>

// Logical actions
typedef enum {
    ACTION_NONE = 0,
    ACTION_MENU_UP,
    ACTION_MENU_DOWN,
    ACTION_MENU_SELECT,
    ACTION_PADDLE_LEFT,
    ACTION_PADDLE_RIGHT,
    ACTION_LAUNCH,
    ACTION_JUMP,
    ACTION_DUCK,
    ACTION_RESTART,
    ACTION_MAX_
} Action;

typedef enum {
    PROGRAM_MENU = 0,
    PROGRAM_BRICKOUT,
    PROGRAM_DINO,
    PROGRAM_ANIMATION,
    PROGRAM_MAX_
} ProgramID;

// Provided by your launcher (stub for now)
ProgramID current_program_id(void);

// Init/update
void input_init(void);
void input_update(uint32_t now_ms);

// Physical button queries
bool input_pressed(int idx);
bool input_released(int idx);
bool input_held(int idx);

// Logical action queries
bool action_pressed(Action a);
bool action_released(Action a);
bool action_held(Action a);

// Universal exit combo
bool exit_combo_triggered(void);
