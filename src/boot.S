# See LICENSE file for copyright and license details.

#include "config.h"
#include "macros.S"

.section .init
/*** This is the kernel's entry point ***/
.globl _start
_start: /* Start of the kernel */
        load_gp
        load_sp

        /* Trap handler interpret mscratch == 0 as trap in machine-mode */
        csrw    mscratch,x0    

        /* No machine-mode preemption */
        csrw    mstatus,x0

        /* Set the trap entry function. */
        la      t0,trap_entry
        csrw    mtvec,t0

        /* make memory accessible */
        csrw    pmpcfg0,0x0f
        li      t0,0xfffffffffffull
        csrw    pmpaddr0,t0

        /* If hartid != MIN_HARTID, then jump to __wait. */
        csrr    t0,mhartid
        li      t1,MIN_HARTID
        bne     t0,t1,_wait

_initialize_kernel: /* Initialize kernel */
        /* Zero bss section */
        la      t0,_bss
        la      t1,_ebss
1:      beq     t0,t1,2f
        sd      x0,0(t0)
        addi    t0,t0,8
        j       1b
2:      
        /* Zero sbss section */
        la      t0,_sbss
        la      t1,_esbss
1:      beq     t0,t1,2f
        sd      x0,0(t0)
        addi    t0,t0,8
        j       1b
2:      

        /* Initializes PCBs. */
        call    proc_init

        /* Print information about kernel, scheduler, and platform. */
        call    print_info

_wake:
        /* Wake cores MIN_HARTID-MAX_HARTID */
        li      t2,1
        li      t0,(MIN_HARTID * 4 + CLINT)
        li      t1,(MAX_HARTID * 4 + CLINT)
1:      sw      t2,0(t0)
        addi    t0,t0,4
        bgeu    t1,t0,1b
       
_wait: 
        /* Wait for initilization to finish */
        csrw    mie,8
        wfi
        csrr    t0,mip
        andi    t0,t0,0x8
        beqz    t0,_wait

        /* Enable timer interrupt */
        li      t0,128
        csrw    mie,t0

        /*** Here we start executing the kernel. ***/
        /*la      ra,trap_resume_proc*/
        tail    sched_start
