#pragma once
#include "proc.h"

void syscall_memory_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_memory_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
