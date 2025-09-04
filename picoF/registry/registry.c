#include "registry.h"

// These symbols are provided by the linker script
extern const ProgramEntry __start_prog_registry[];
extern const ProgramEntry __stop_prog_registry[];

// No runtime code needed — all handled via linker sections
