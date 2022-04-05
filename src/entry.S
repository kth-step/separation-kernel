# See LICENSE file for copyright and license details.

.globl AsmTrapEntry
.globl AsmTrapExit

.section .text.entry
.balign 16
AsmTrapEntry:
        /* Load kernel process sp, store user sp. */
        csrrw   sp,mscratch,sp
#ifdef DEBUG
        /* If mscratch is 0, we have bug in the kernel */
        beqz    sp,__hang
#endif
        /* Store user gp, tp registers. */
        sd      gp,(0*8)(sp)
        sd      tp,(1*8)(sp)
        /* Load kernel gp and tp. */
        ld      gp,(-1*8)(sp)   /* points to kernel global data */
        ld      tp,(-2*8)(sp)   /* points to current pcb */
        /* Store user t0-t6, ra registers. */
        sd      t0,(2*8)(sp)
        sd      t1,(3*8)(sp)
        sd      t2,(4*8)(sp)
        sd      t3,(5*8)(sp)
        sd      t4,(6*8)(sp)
        sd      t5,(7*8)(sp)
        sd      t6,(8*8)(sp)
        sd      ra,(9*8)(sp)
#ifdef DEBUG
        /* Write user sp to stack. */
        /* In non-debug mode, user-sp is stored in switch.S. */
        csrrw   t3,mscratch,zero
        sd      t3,(10*8)(sp)
#endif

        /* Save user a0-a7. */
        sd      a0,(3*8)(tp)
        sd      a1,(4*8)(tp)
        sd      a2,(5*8)(tp)
        sd      a3,(6*8)(tp)
        sd      a4,(7*8)(tp)
        sd      a5,(8*8)(tp)
        sd      a6,(9*8)(tp)
        sd      a7,(10*8)(tp)
        /* Get user pc, saved later as we might modify it. */
        csrr    t3,mepc

        /* Check trap type. */
        csrr    t1,mcause
        /* If msb is 0, then we have an exception/syscall */
        bge     t1,zero,1f

__interrupt:
        /* Load interrupt trampoline address */
        la      ra,__interrupt_trampoline
        j       2f

        /* If mcause == 8, then syscall, else exception. */
1:      li      t2,8
        bne     t1,t2,__exception

__syscall:
        /* Number of syscall is 16 for now. */
        li      t2,16
        /* If t0 >= 16, invalid syscall */
        bge     t0,t2,__exception
        /* Load address to syscall trampoline */
        la      ra,__syscall_trampoline
        /* Adds 4 to pc (jumping over ecall instruction */
        addi    t3,t3,4
        /* Make offset to trampoline address */
        slli    t1,t0,2
        j       3f

__exception:
        la      ra,__exception_trampoline
        /* Set a0 to mcause */
        mv      a0,t1
        /* Set a1 to mtval */
        csrr    a1,mtval
        /* Set a2 to mepc */
        mv      a2,t3
        /* Make offset to trampoline address. */
2:      slli    t1,t1,2
        /* Add offset to trampine base. */
3:      add     ra,ra,t1
        /* Store user pc */
        sd      t3,(11*8)(tp)
        /* Jump to trampoline */
        jalr    ra,0(ra)
        /* Check if we have a timer interrupt. */
        csrr    t1,mip
        andi    t1,t1,128
        /* If we have the interrupt, jump to AsmSwitchToSched, 
         * triggering the scheduler. Return address will be
         * the csrr above.
         */
        bnez    t1,AsmSwitchToSched

AsmTrapExit:
#ifdef DEBUG
        /* Write user sp to mscratch. */
        ld      t0,(10*8)(sp)
        csrw    mscratch,
#endif
        /* Write user pc to mepc. */
        ld      t3,(11*8)(tp)
        csrw    mepc,t3
        /* Load user a0-a7. */
        ld      a7,(10*8)(tp)
        ld      a6,(9*8)(tp)
        ld      a5,(8*8)(tp)
        ld      a4,(7*8)(tp)
        ld      a3,(6*8)(tp)
        ld      a2,(5*8)(tp)
        ld      a1,(4*8)(tp)
        ld      a0,(3*8)(tp)
        /* Load process ra,t0-t6 register. */
        ld      ra,(9*8)(sp)
        ld      t6,(8*8)(sp)
        ld      t5,(7*8)(sp)
        ld      t4,(6*8)(sp)
        ld      t3,(5*8)(sp)
        ld      t2,(4*8)(sp)
        ld      t1,(3*8)(sp)
        ld      t0,(2*8)(sp)
        /* Store kernel gp and tp. */
        sd      tp,(-2*8)(sp)
        sd      gp,(-1*8)(sp)
        /* Load user gp and tp. */
        ld      tp,(1*8)(sp)
        ld      gp,(0*8)(sp)
        /* Load process sp, store kernel sp. */
        csrrw   sp,mscratch,sp
        /* Go to user mode. */
        mret

__exception_trampoline:
        .option push
        .option norvc   /* Prevent compressed instructions */
        /* Exceptions */
        j       __hang  /* 0: Instruction address misaligned */
        j       __hang  /* 1: Instruction access fault */
        j       __hang  /* 2: Illegal instruction */
        j       __hang  /* 3: Breakpoint */
        j       __hang  /* 4: Load address misaligned */
        j       __hang  /* 5: Load access fault */
        j       __hang  /* 6: Store/AMO address misaligned */
        j       __hang  /* 7: Store/AMO access fault */
        j       __hang  /* 8: Environment call from U-mode */
        j       __hang  /* 9: Environment call from S-mode */
        j       __hang  /* 10: Reserved */
        j       __hang  /* 11: Environment call from M-mode */
        j       __hang  /* 12: Instruction page fault */
        j       __hang  /* 13: Load page fault */
        j       __hang  /* 14: Reserved */
        j       __hang  /* 15: Store/AMO page fault */
__interrupt_trampoline:
        /* Interrupts */
        j       __hang  /*  0: User software interrupt */
        j       __hang  /*  1: Supervisor software interrupt */
        j       __hang  /*  2: Reserved */
        j       __hang  /*  3: Machine software interrupt */
        j       __hang  /*  4: User timer interrupt */
        j       __hang  /*  5: Supervisor timer interrupt */
        j       __hang  /*  6: Reserved */
        j       AsmSwitchToSched /*  7: Machine timer interrupt */
        j       __hang  /*  8: User external interrupt */
        j       __hang  /*  9: Supervisor external interrupt */
        j       __hang  /* 10: Reserved */
        j       __hang  /* 11: Machine external interrupt */
__syscall_trampoline:
        /* System calls */
        j       EcallPid
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        .option pop

__hang: 
        j       __hang
