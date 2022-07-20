#pragma once
#include "proc.h"

void syscall_handle_memory(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_handle_pmp(registers_t* regs, cap_node_t* cn, cap_t cap);
