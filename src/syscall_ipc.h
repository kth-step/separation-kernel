#pragma once

#include "proc.h"
#include "proc_state.h"

void syscall_channels_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_channels_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_receiver_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_receiver_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_receiver_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_sender_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
