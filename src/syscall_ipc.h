#pragma once

#include "proc.h"

void syscall_handle_channels(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_handle_sender(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_handle_receiver(registers_t* regs, cap_node_t* cn, cap_t cap);
