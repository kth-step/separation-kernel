// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "proc.h"

void syscall_get_pid(void) __attribute__((noreturn));
void syscall_read_reg(void) __attribute__((noreturn));
void syscall_write_reg(void) __attribute__((noreturn));
void syscall_yield(void) __attribute__((noreturn));
void syscall_read_cap(void) __attribute__((noreturn));
void syscall_move_cap(void) __attribute__((noreturn));
void syscall_delete_cap(void) __attribute__((noreturn));
void syscall_revoke_cap(void) __attribute__((noreturn));
void syscall_invoke_cap(void) __attribute__((noreturn));
void syscall_derive_cap(void) __attribute__((noreturn));

void exception_handler(uint64_t mcause, uint64_t mtval) __attribute__((noreturn));
void illegal_instruction_handler(uint64_t mcause, uint64_t mtval) __attribute__((noreturn));

void trap_entry(void) __attribute__((noreturn));
void trap_resume_proc(void) __attribute__((noreturn));
void trap_yield(void) __attribute__((noreturn));
void trap_syscall_exit(uint64_t code) __attribute__((noreturn));
void trap_syscall_exit2(uint64_t code) __attribute__((noreturn));
void trap_syscall_exit3(void) __attribute__((noreturn));
