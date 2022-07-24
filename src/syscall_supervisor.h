#pragma once

#include "proc.h"

void syscall_supervisor_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_suspend(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_resume(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_get_state(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_read_reg(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_write_reg(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_read_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_give_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
void syscall_supervisor_take_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
