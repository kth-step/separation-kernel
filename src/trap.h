#pragma once
#include "proc.h"
#include "types.h"

void syscall_handler(struct registers* regs, uint64_t mcause, uint64_t mtval);

void exception_handler(struct registers* regs, uint64_t mcause, uint64_t mtval);

void trap_entry(void) __attribute__((noreturn));
void trap_resume_proc(void) __attribute__((noreturn));
void trap_yield(void);
