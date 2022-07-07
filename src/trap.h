#pragma once
#include "types.h"
#include "proc.h"

void SyscallHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval);

void ExceptionHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval);
