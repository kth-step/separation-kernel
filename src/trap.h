#pragma once
#include "types.h"
#include "proc.h"

void SyscallHandler(struct registers *regs, uint64_t mcause, uint64_t mtval);

void ExceptionHandler(struct registers *regs, uint64_t mcause, uint64_t mtval);

void trap_entry(void) __attribute__((noreturn));
void trap_resume_proc(void) __attribute__((noreturn));
void trap_recv_yield(void);
