#pragma once

#include "proc.h"

void syscall_supervisor_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
