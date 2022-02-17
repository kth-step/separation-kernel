#include "config.h"
#include "proc.h"

Process processes[N_PROC];
uintptr_t stack[N_PROC][STACK_SIZE];
