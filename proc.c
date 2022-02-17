#include "config.h"
#include "proc.h"

Process processes[N_PROC];
uint8_t stack[N_PROC][STACK_SIZE];
