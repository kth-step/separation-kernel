#pragma once
#include "proc.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void syscall_handler(registers_t* regs);

void exception_handler(registers_t* regs, uint64_t mcause, uint64_t mtval);

void trap_entry(void) __attribute__((noreturn));
void trap_resume_proc(void) __attribute__((noreturn));
void trap_yield(void) __attribute__((noreturn));
