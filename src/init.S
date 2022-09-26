# See LICENSE file for copyright and license details.

#include "macros.S"

.globl _start
.globl hang

.section .init
/*** This is the kernel's entry point ***/
_start: /* Start of the kernel */
        load_gp
        load_sp

        /* Trap handler interpret mscratch == 0 as trap in machine-mode */
        csrw    mscratch,x0    

        /* No machine-mode preemption */
        csrw    mstatus,x0

        /* Set the trap entry function. */
        lla     t0,trap_entry
        csrw    mtvec,t0

        /* make memory accessible */
        csrw    pmpcfg0,0x0f
        li      t0,0xfffffffffffull
        csrw    pmpaddr0,t0

        /* If hartid != MIN_HARTID, then jump to __wait. */
        csrr    t0,mhartid
        li      t1,MIN_HARTID
        bne     t0,t1,wait

initialize_kernel: /*** Initialize kernel ***/
        /* Zero bss section */
        lla     t0,_bss
        lla     t1,_ebss
        j       2f
1:      sw      x0,0(t0)
        addi    t0,t0,4
2:      bne     t0,t1,1b

        /* Initializes PCBs. */
        call    proc_init
        call    sched_init

#ifndef NDEBUG
        /* Print information about kernel, scheduler, and platform. */
        call    print_info
#endif

wake:   /*** Wake cores MIN_HARTID .. MAX_HARTID. ***/
        li      t2,1
        li      t0,(MIN_HARTID * 4 + CLINT)
        li      t1,(MAX_HARTID * 4 + CLINT)
1:      sw      t2,0(t0)
        addi    t0,t0,4
        bgeu    t1,t0,1b
       
wait:   /*** Wait for initilization to finish. ***/
        csrw    mie,(1 << 3)
        wfi
        csrr    t0,mip
        andi    t0,t0,(1 << 3)
        beqz    t0,wait

        /* Disable software interrupt. */
        li      t0,CLINT
        csrr    t1,mhartid
        slli    t1,t1,2
        add     t0,t0,t1
        sw      x0,(t0)

        /* Enable timer interrupt. */
        li      t0,128
        csrw    mie,t0

        /*** Here we start the processes. ***/
        la      ra,trap_resume_proc
        tail    sched_start

hang:
        j       hang
