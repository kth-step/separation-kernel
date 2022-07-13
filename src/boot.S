# See LICENSE file for copyright and license details.

#include "config.h"
#include "macros.S"

.section .init
/*** This is the kernel's entry point ***/
.globl _start
_start:
        load_gp_sp

        csrw    mscratch,x0    

        /* Set the trap entry function. */
        la      t0,trap_entry
        ori     t0,t0,1
        csrw    mtvec,t0

        /* Enable machine timer interrupt. */
        li      t0,128
        csrw    mie,t0

        /* make all memory accessible */
        csrw    pmpcfg0,0x0f
        li      t0,0xfffffffffffull
        csrw    pmpaddr0,t0

        /* If hartid != 0, then jump to __wait. */
        csrr    t0,mhartid
        bnez    t0,__wait

        /* Single process work*/
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

        call    proc_init

        la      a0,msg_done
        call    kprint

        /* Unlock the other harts. */
        fence   rw,rw
        la      t0,lock
        sd      x0,(t0)

       
        /* Wait for hart 0 to finish process initialization. */
__wait: 
        la      t0,lock
        ld      t1,(t0)
        bnez    t1,__wait

        csrr    t0,mhartid
        li      t1,MIN_HARTID
        li      t2,MAX_HARTID
        blt     t0,t1,__boot_park
        blt     t2,t0,__boot_park

        /*** Here we start executing the kernel. ***/
        /* Set thread pointer to 0. */
        li      tp,0
        /* Jump to scheduler. */
        la      ra,trap_resume_proc
        tail    sched

__boot_park:
        wfi
        j       __boot_park

.section .data.init
/* This lock is used for process initialization */
lock: .dword 1

msg_init: .string "s3k init\r\n"
msg_done: .string "s3k init done\r\n"
