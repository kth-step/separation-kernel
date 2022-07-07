# See LICENSE file for copyright and license details.

#include "config.h"


.section .init
/*** This is the kernel's entry point ***/
.globl _start
_start:
        csrr    t0,mhartid
        li      t1,N_CORES
        bgt     t0,t1,__hang

        li      x1,0
        li      x2,0
        li      x3,0
        li      x4,0
        li      x5,0
        li      x6,0
        li      x7,0
        li      x8,0
        li      x9,0
        li      x10,0
        li      x11,0
        li      x12,0
        li      x13,0
        li      x14,0
        li      x15,0
        li      x16,0
        li      x17,0
        li      x18,0
        li      x19,0
        li      x20,0
        li      x21,0
        li      x22,0
        li      x23,0
        li      x24,0
        li      x25,0
        li      x26,0
        li      x27,0
        li      x28,0
        li      x29,0
        li      x30,0
        li      x31,0
        csrw    mstatus,x0
        csrw    medeleg,x0
        csrw    mideleg,x0
        csrw    satp,x0
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
        li      t3,MIN_HARTID
        blt     t0,t3,__hang
        sub     t0,t0,t3
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
        csrw    mscratch,x0
        /* Enable machine timer interrupt. */
        li      t0,128
        csrw    mie,t0
        /* Set mstatus to 0, so we go to user-mode when executing mret. */
        csrw    mstatus,x0

        csrw    pmpcfg0,0x0f
        li      t0,0xfffffffffffull
        csrw    pmpaddr0,t0

        /* If hartid != 0, then jump to __wait. */
        csrr    t0,mhartid
        li      t1,MIN_HARTID
        bne     t1,t0,__wait


        la      t0,_bss
        la      t1,_ebss
        j       2f
1:      sd      x0,0(t0)
        addi    t0,t0,8
2:      bne     t0,t1,1b

        la      t0,_sbss
        la      t1,_esbss
        j       2f
1:      sd      x0,0(t0)
        addi    t0,t0,8
2:      bne     t0,t1,1b
        #if QEMU_DEBUGGING == 0
                call    uart_init
                la      a0,msg_init
                call    uart_puts
        #endif

        /* Initialize the process, only hart 0 does this. */
        call    ProcInitProcesses
        #if QEMU_DEBUGGING == 0
                la      a0,msg_done
                call    uart_puts
        #endif

        /* Unlock the other harts. */
        fence   rw,rw
        la      t0,lock
        sd      x0,(t0)
        /* Jump to 1 */
        j       1f
       
        /* Wait for hart 0 to finish process initialization. */
__wait: 
        la      t0,lock
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

msg_init: .string "s3k init\r\n"
msg_done: .string "s3k init done\r\n"