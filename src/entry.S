# See LICENSE file for copyright and license details.

.globl AsmTrapEntry
.globl AsmTrapExit

.section .text.entry
.balign 16
AsmTrapEntry:
        /* Load kernel sp, store user sp. */
        csrrw   sp,mscratch,sp
        beqz    sp,__hang
        /* Store user gp, tp registers. */
        sd      gp,(0*8)(sp)
        sd      tp,(1*8)(sp)    /* Load tp early for optimization */
        /* Load kernel gp and tp. */
        ld      gp,(-1*8)(sp)
        ld      tp,(-2*8)(sp)
        /* Store user t0-t6, ra registers. */
        sd      t0,(2*8)(sp)
        sd      t1,(3*8)(sp)
        sd      t2,(4*8)(sp)
        sd      t3,(5*8)(sp)
        sd      t4,(6*8)(sp)
        sd      t5,(7*8)(sp)
        sd      t6,(8*8)(sp)
        sd      ra,(9*8)(sp)
        csrrw   t3,mscratch,zero
        sd      t3,(10*8)(sp)

        /* Save user a0-a7. */
        sd      a0,(3*8)(tp)
        sd      a1,(4*8)(tp)
        sd      a2,(5*8)(tp)
        sd      a3,(6*8)(tp)
        sd      a4,(7*8)(tp)
        sd      a5,(8*8)(tp)
        sd      a6,(9*8)(tp)
        sd      a7,(10*8)(tp)
        csrr    t3,mepc

        /* Check trap type. */
        csrr    t1,mcause
        bge     t1,zero,1f

__interrupt:
        la      ra,__interrupt_trampoline
        j       2f

1:      li      t2,8
        bne     t1,t2,__exception

__syscall:
        li      t2,16
        bge     t0,t2,__exception
        la      ra,__syscall_trampoline
        addi    t3,t3,4
        slli    t1,t0,2
        j       3f

__exception:
        la      ra,__exception_trampoline
        mv      a0,t1
        csrr    a1,mtval
        mv      a2,t3

        /* Calculate trampoline address */
2:      slli    t1,t1,2
3:      add     ra,ra,t1
        /* Store user pc */
        sd      t3,(11*8)(tp)
        /* Jump to trampoline */
        jalr    ra,0(ra)
        csrr    t1,mip
        andi    t1,t1,128
        bnez    t1,AsmSwitchToSched

AsmTrapExit:
        ld      t0,(10*8)(sp)
        csrw    mscratch,t0
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
