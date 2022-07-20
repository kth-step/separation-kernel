#pragma once

#include "proc.h"

void syscall_handle_supervisor(registers_t* regs, cap_node_t* cn, cap_t cap);
