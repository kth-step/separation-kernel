# See LICENSE file for copyright and license details.

#include "config.h"


.section .init
/*** This is the kernel's entry point ***/
.globl _start
_start:
        csrr    t0,mhartid
        li      t1,N_CORES
        bge     t0,t1,__hang // Assmues we are in QEMU. Would be bgt on board.
        /*** Load the global pointer. ***/
        /* We load gp first so we can used relaxed (gp-relative) addressing. */
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop

        /*** Initialize the core/hart stack pointer. ***/
        /* Load address to core_stack, defined in stach.h. */
        la      sp,core_stack
        /* Get hartid+1. */
        csrr    t0,mhartid
        addi    t0,t0,1
        /* Get an offset by multiplying hartid+1 with STACK_SIZE. */
        slli    t0,t0,LOG_STACK_SIZE
        /* Add offset to core_stack, sp should now point at the top of the
         * core's stack. */
        add     sp,sp,t0

        /*** Initialize core's control and status registers. ***/
        /* Set the trap entry function. */
        la      t0,AsmTrapEntry
        ori     t0,t0,1
        csrw    mtvec,t0
        /* Enable machine timer interrupt. */
        li      t0,128
        csrw    mie,t0
        /* Set mstatus to 0, so we go to user-mode when executing mret. */
        csrw    mstatus,x0

        csrw    pmpcfg0,0x0f
        li      t0,-1
        csrw    pmpaddr0,t0

        /* If hartid != 0, then jump to __wait. */
        csrr    t0,mhartid
        bnez    t0,__wait

        /* Initialize the process, only hart 0 does this. */
        call    ProcInitProcesses

        /* Unlock the other harts. */
        fence   rw,rw
        la      t0,lock
        sd      x0,(t0)
        /* Jump to 1 */
        j       1f
       
        /* Wait for hart 0 to finish process initialization. */
__wait: la      t0,lock
        ld      t1,(t0)
        bnez    t1,__wait


        /*** Here we start executing the kernel. ***/
1:      /* Set thread pointer to 0. */
        li      tp,0
        /* Jump to scheduler. */
        call    Sched
        tail    AsmResumeProc

__hang:
        wfi
        j       __hang

.section .data.init
/* This lock is used for process initialization */
lock: .dword 1
