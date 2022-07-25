#pragma once

#include "proc.h"

void syscall_supervisor_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
