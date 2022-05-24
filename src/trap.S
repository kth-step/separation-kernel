# See LICENSE file for copyright and license details.

.globl AsmTrapEntry
.globl AsmResumeProc

.section .text.entry
.balign 16
.option push
.option norvc   /* Prevent compressed instructions */
AsmTrapEntry:
        /* Interrupts */
        j       __exception_handler  /*  0: User software interrupt */
        j       __hang  /*  1: Supervisor software interrupt */
        j       __hang  /*  2: Reserved */
        j       __hang  /*  3: Machine software interrupt */
        j       __hang  /*  4: User timer interrupt */
        j       __hang  /*  5: Supervisor timer interrupt */
        j       __hang  /*  6: Reserved */
        j       __timer_handler /*  7: Machine timer interrupt */
        j       __hang  /*  8: User external interrupt */
        j       __hang  /*  9: Supervisor external interrupt */
        j       __hang  /* 10: Reserved */
        j       __hang  /* 11: Machine external interrupt */

__exception_trampoline:
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
__syscall_trampoline:
        /* System calls */
        j       SyscallReadPid
        j       SyscallCapRead
        j       SyscallCapMove
        j       SyscallCapRevoke
        j       SyscallCapDelete
        j       SyscallMemorySlice
        j       SyscallMemorySplit
        j       SyscallTimeSlice
        j       SyscallTimeSplit
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
        j       __hang
.option pop

__exception_handler:
        /* Load kernel process sp, store user sp. */
        csrrw   sp,mscratch,sp
#ifdef DEBUG
        /* If mscratch is 0, we have bug in the kernel */
        beqz    sp,__hang
#endif
        addi    sp,sp,-32*8
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

        /* Write user sp to stack. */
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
        /* Get user pc, saved later as we might modify it. */
        csrr    t3,mepc

        /* Check trap type. */
        csrr    t1,mcause

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
        /* Jump to trampoline */
        j       __trampoline

__exception:
        la      ra,__exception_trampoline
        /* Set a0 to mcause */
        mv      a0,t1
        /* Set a1 to mtval */
        csrr    a1,mtval
        /* Set a2 to mepc */
        mv      a2,t3
        /* Make offset to trampoline address. */
        slli    t1,t1,2

__trampoline:
        /* Store user pc */
        sd      t3,(11*8)(sp)
        csrsi   mstatus,1
        /* Jump to trampoline */
        add     ra,ra,t1
        jalr    ra,0(ra)
        csrci   mstatus,1

__exception_exit:
        /* Write user sp to mscratch. */
        ld      t0,(10*8)(sp)
        csrw    mscratch,t0
        /* Write user pc to mepc. */
        ld      t3,(11*8)(sp)
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
        addi    sp,sp,32*8
        csrrw   sp,mscratch,sp
        /* Go to user mode. */
        mret

__hang: 
        j       __hang

__timer_handler:
        csrrw   sp,mscratch,sp
        bnez    sp,1f
        csrrw   sp,mscratch,zero
        addi    sp,sp,(-32*8)
        sd      gp,(3*8)(sp)
        sd      tp,(4*8)(sp)
        j       2f
1:      
        addi    sp,sp,(-32*8)
        sd      gp,(3*8)(sp)
        sd      tp,(4*8)(sp)
        ld      gp,(-1*8)(sp)
        ld      tp,(-2*8)(sp)
2:
        sd      ra,(1*8)(sp)
        sd      t0,(5*8)(sp)
        sd      t1,(6*8)(sp)
        sd      t2,(7*8)(sp)
        sd      s0,(8*8)(sp)
        sd      s1,(9*8)(sp)
        sd      a0,(10*8)(sp)
        sd      a1,(11*8)(sp)
        sd      a2,(12*8)(sp)
        sd      a3,(13*8)(sp)
        sd      a4,(14*8)(sp)
        sd      a5,(15*8)(sp)
        sd      a6,(16*8)(sp)
        sd      a7,(17*8)(sp)
        sd      s2,(18*8)(sp)
        sd      s3,(19*8)(sp)
        sd      s4,(20*8)(sp)
        sd      s5,(21*8)(sp)
        sd      s6,(22*8)(sp)
        sd      s7,(23*8)(sp)
        sd      s8,(24*8)(sp)
        sd      s9,(25*8)(sp)
        sd      s10,(26*8)(sp)
        sd      s11,(27*8)(sp)
        sd      t3,(28*8)(sp)
        sd      t4,(29*8)(sp)
        sd      t5,(30*8)(sp)
        sd      t6,(31*8)(sp)

        /* Store PC */
        csrr    t1,mepc
        sd      t1,(0*8)(sp)

        /* Store SP */
        csrr    t0,mscratch
        sd      t0,(2*8)(sp)

        /* Swap core and process sp */
        amoswap.d.aqrl  sp,sp,(tp)

        /* Call scheduler */
        call    Sched
        
AsmResumeProc:
        /* Swap core and process sp */
        amoswap.d.aqrl  sp,sp,(tp)

        /* Save kernel gp and TP */
        sd      gp,(-1*8)(sp)
        sd      tp,(-2*8)(sp)

        /* Load PC */
        ld      t1,(2*8)(sp)
        csrw    mscratch,t1

        /* Load SP */
        ld      t0,(0*8)(sp)
        csrw    mepc,t0

        ld      t6,(31*8)(sp)
        ld      t5,(30*8)(sp)
        ld      t4,(29*8)(sp)
        ld      t3,(28*8)(sp)
        ld      s11,(27*8)(sp)
        ld      s10,(26*8)(sp)
        ld      s9,(25*8)(sp)
        ld      s8,(24*8)(sp)
        ld      s7,(23*8)(sp)
        ld      s6,(22*8)(sp)
        ld      s5,(21*8)(sp)
        ld      s4,(20*8)(sp)
        ld      s3,(19*8)(sp)
        ld      s2,(18*8)(sp)
        ld      a7,(17*8)(sp)
        ld      a6,(16*8)(sp)
        ld      a5,(15*8)(sp)
        ld      a4,(14*8)(sp)
        ld      a3,(13*8)(sp)
        ld      a2,(12*8)(sp)
        ld      a1,(11*8)(sp)
        ld      a0,(10*8)(sp)
        ld      s1,(9*8)(sp)
        ld      s0,(8*8)(sp)
        ld      t2,(7*8)(sp)
        ld      t1,(6*8)(sp)
        ld      t0,(5*8)(sp)
        ld      tp,(4*8)(sp)
        ld      gp,(3*8)(sp)
        ld      ra,(1*8)(sp)
        addi    sp,sp,32*8
        csrrw   sp,mscratch,sp
        mret
