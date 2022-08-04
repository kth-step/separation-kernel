#pragma once

#include "proc.h"
#include "proc_state.h"

void syscall_receiver_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_sender_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_server_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_client_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
