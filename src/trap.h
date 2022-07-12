#pragma once
#include "types.h"
#include "proc.h"

void SyscallHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval);

void ExceptionHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval);

void trap_entry(void) __attribute__((noreturn));
void trap_resume_proc(void) __attribute__((noreturn));
void trap_recv_yield(void) __attribute__((noreturn));
