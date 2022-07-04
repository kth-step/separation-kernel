#pragma once
#include "types.h"

typedef struct trap_frame {
        uint64_t pc;
        uint64_t ra;
        uint64_t sp;
        uint64_t gp;
        uint64_t tp;
        uint64_t t0;
        uint64_t t1;
        uint64_t t2;
        uint64_t t3;
        uint64_t t4;
        uint64_t t5;
        uint64_t t6;
        uint64_t a0;
        uint64_t a1;
        uint64_t a2;
        uint64_t a3;
        uint64_t a4;
        uint64_t a5;
        uint64_t a6;
        uint64_t a7;
        uint64_t mcause;
        uint64_t mtval;
} TrapFrame;

void syscall_handler(TrapFrame *tf);
