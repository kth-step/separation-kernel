# See LICENSE file for copyright and license details.
#include "inc/config.h"

# Boot code {{{
.section .init
.extern stack
.globl _start
_start:

        # Initialize global pointer and stack pointer.
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop
        csrr    t0,mhartid
        addi    t0,t0,1
        li      t1,STACK_SIZE
        mul     t0,t0,t1
        la      sp,core_stack
        add     sp,sp,t0
        
        # Remove memory protection
        li      t0,0xF
        li      t1,-1
        csrw    pmpcfg0,t0
        csrw    pmpaddr0,t1

        # Setup trap handler 
        la      t0,AsmTrapHandler
        csrw    mtvec,t0
        csrw    mstatus,x0
        li      t0,(1 << 7)
        csrw    mie,t0

        csrr    t0,mhartid
        bnez    t0,1f
        call    init
1:      ld      t1,init_lock
        bge     t0,t1,1b
        tail    sched
# }}}
