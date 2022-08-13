// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "proc.h"

void syscall_unimplemented(void) __attribute__((noreturn));
void syscall_read_cap(cap_node_t* cap_node, cap_t cap) __attribute__((noreturn));
void syscall_move_cap(cap_node_t* cap_node, cap_t cap) __attribute__((noreturn));
void syscall_delete_cap(cap_node_t* cap_node, cap_t cap) __attribute__((noreturn));
void syscall_revoke_cap(cap_node_t* cap_node, cap_t cap) __attribute__((noreturn));
void syscall_invoke_cap(cap_node_t* cap_node, cap_t cap) __attribute__((noreturn));
void syscall_derive_cap(cap_node_t* cap_node, cap_t cap) __attribute__((noreturn));
void syscall_get_pid(void) __attribute__((noreturn));
void syscall_read_reg(void) __attribute__((noreturn));
void syscall_write_reg(void) __attribute__((noreturn));
void syscall_yield(void) __attribute__((noreturn));

void exception_handler(void) __attribute__((noreturn));

void trap_entry(void) __attribute__((noreturn));
void trap_resume_proc(void) __attribute__((noreturn));
void trap_syscall_exit1(uint64_t code) __attribute__((noreturn));
void trap_syscall_exit2(uint64_t code) __attribute__((noreturn));
