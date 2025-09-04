#include "registry.h"
#include "input/input.h" // for ProgramID

extern const ProgramEntry __start_prog_registry[];
extern const ProgramEntry __stop_prog_registry[];

// Track active program
static ProgramID active_program = PROGRAM_MENU;

void registry_set_active_program(ProgramID p) {
    active_program = p;
}

ProgramID current_program_id(void) {
    return active_program;
}
