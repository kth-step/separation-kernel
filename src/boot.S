#include "config.h"

.globl _start

.section .init

_start:
        // Initialize the global pointer
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop

        // Initialize the core stack pointer
        la      sp,core_stack
        csrr    t0,mhartid
        addi    t0,t0,1
        slli    t0,t0,LOG_STACK_SIZE
        add     sp,sp,t0

        // Initialize core csr
        la      t0,AsmTrapEntry
        csrw    mtvec,t0
        li      t0,128
        csrw    mie,t0

        csrr    t0,mhartid
        bnez    t0,1f

        call    InitProc

        fence   rw,rw
        la      t0,lock
        sd      zero,(t0)
       
1:      la      t0,lock
        ld      t1,(t0)
        bnez    t1,1b
        la      ra,AsmSwitchToProc
        li      tp,0
        tail    Sched

.section .data.init
lock: .dword 1
