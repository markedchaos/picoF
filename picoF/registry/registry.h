#ifndef REGISTRY_H
#define REGISTRY_H

#include <stdint.h>

typedef void (*ProgramFunc)(void);

typedef struct {
    const char *name;       // Display name shown in the launcher
    ProgramFunc run;        // Function to run the program (run_##ID)
    const uint8_t *icon;    // Optional icon data (can be NULL)
} ProgramEntry;

// Register a program using a C identifier (ID), a display string, and an icon pointer.
// Example:
//   REGISTER_PROGRAM(dino, "Dino", NULL);
//   // Declares: void run_dino(void);
//   // Emits registry entry with name="Dino", run=run_dino, icon=NULL
#define REGISTER_PROGRAM(ID, DISPLAY, ICON) \
    void run_##ID(void); \
    __attribute__((used, section("prog_registry"))) \
    static const ProgramEntry _reg_##ID = { DISPLAY, run_##ID, ICON };

// Accessors backed by GNU ld start/stop section symbols
extern const ProgramEntry __start_prog_registry[];
extern const ProgramEntry __stop_prog_registry[];

static inline uint32_t registry_count(void) {
    return (uint32_t)(__stop_prog_registry - __start_prog_registry);
}

static inline const ProgramEntry* registry_entry(uint32_t idx) {
    return &__start_prog_registry[idx];
}
// --- Added for program-aware input mapping ---
#include "input/input.h" // for ProgramID enum

void registry_set_active_program(ProgramID p);
ProgramID current_program_id(void);

#endif // REGISTRY_H
