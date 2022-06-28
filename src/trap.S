# See LICENSE file for copyright and license details.
#include "config.h"

.globl AsmTrapEntry
.globl AsmResumeProc

.option push
.option norvc   /* Prevent compressed instructions */
.align 8
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
.option pop

__exception_handler:
        /* Load kernel process sp, store user sp. */
        csrrw   sp,mscratch,sp
#ifdef DEBUG
        /* If mscratch is 0, we have bug in the kernel */
        beqz    sp,__mhang
#endif
        /* Store user gp, tp registers. */
        sd      gp,(-3*8)(sp)
        sd      tp,(-4*8)(sp)
        /* Load kernel gp and tp. */
        ld      gp,(-1*8)(sp)   /* points to kernel global data */
        ld      tp,(-2*8)(sp)   /* points to current pcb */
        /* Store user t0-t6, ra registers. */
        sd      t0,(-5*8)(sp)
        sd      t1,(-6*8)(sp)
        sd      t2,(-7*8)(sp)
        sd      t3,(-8*8)(sp)
        sd      t4,(-9*8)(sp)
        sd      t5,(-10*8)(sp)
        sd      t6,(-11*8)(sp)
        sd      ra,(-12*8)(sp)

        /* Write user sp to stack. */
        csrrw   t3,mscratch,zero
        sd      t3,(-13*8)(sp)

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
        /* Load address to syscall trampoline */
        la      ra,SyscallHandler
        /* Adds 4 to pc (jumping over ecall instruction */
        addi    t3,t3,4
        /* Store user pc */
        sd      t3,(-14*8)(sp)

        /* Fix sp and enable preemption */
        addi    sp,sp,-14*8
        csrwi   mstatus,8

        call    SyscallHandler

        /* Fix sp and disable preemption */
        csrwi   mstatus,0
        addi    sp,sp,14*8

        j       __exception_exit

__exception:
        j       __hang
        la      ra,__exception_trampoline
        /* Set a0 to mcause */
        mv      a0,t1
        /* Set a1 to mtval */
        csrr    a1,mtval
        /* Set a2 to mepc */
        mv      a2,t3
        /* Make offset to trampoline address. */
        slli    t1,t1,2

        /* Store user pc */
        sd      t3,(-14*8)(sp)

        addi    sp,sp,-14*8
        csrwi   mstatus,8

        /* Jump to trampoline */
        add     ra,ra,t1
        jalr    ra,0(ra)

        /* Fix sp and disable preemption */
        csrwi   mstatus,0
        addi    sp,sp,14*8

__exception_exit:
        /* Write user pc to mepc. */
        ld      t3,(-14*8)(sp)
        beqz    t3,__hang
        csrw    mepc,t3
        /* Write user sp to mscratch. */
        ld      t0,(-13*8)(sp)
        csrw    mscratch,t0
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
        ld      ra,(-12*8)(sp)
        ld      t6,(-11*8)(sp)
        ld      t5,(-10*8)(sp)
        ld      t4,(-9*8)(sp)
        ld      t3,(-8*8)(sp)
        ld      t2,(-7*8)(sp)
        ld      t1,(-6*8)(sp)
        ld      t0,(-5*8)(sp)
        /* Store kernel gp and tp. */
        sd      tp,(-2*8)(sp)
        sd      gp,(-1*8)(sp)
        /* Load user gp and tp. */
        ld      tp,(-4*8)(sp)
        ld      gp,(-3*8)(sp)
        /* Load process sp, store kernel sp. */
        csrrw   sp,mscratch,sp
        /* Go to user mode. */
        mret

__hang: 
        j       __hang
__mhang: 
        j       __mhang

__timer_handler:
        csrrw   sp,mscratch,sp
        bnez    sp,1f
__timer_handler_machine:
        csrr    sp,mscratch
        /* Store kernel gp, tp */
        sd      gp,(-1*8)(sp)
        sd      tp,(-2*8)(sp)
1:      
        sd      ra,(-4*8)(sp)
        //sd      sp,(-5*8)(sp)
        sd      gp,(-6*8)(sp)
        sd      tp,(-7*8)(sp)
        sd      t0,(-8*8)(sp)
        sd      t1,(-9*8)(sp)
        sd      t2,(-10*8)(sp)
        sd      s0,(-11*8)(sp)
        sd      s1,(-12*8)(sp)
        sd      a0,(-13*8)(sp)
        sd      a1,(-14*8)(sp)
        sd      a2,(-15*8)(sp)
        sd      a3,(-16*8)(sp)
        sd      a4,(-17*8)(sp)
        sd      a5,(-18*8)(sp)
        sd      a6,(-19*8)(sp)
        sd      a7,(-20*8)(sp)
        sd      s2,(-21*8)(sp)
        sd      s3,(-22*8)(sp)
        sd      s4,(-23*8)(sp)
        sd      s5,(-24*8)(sp)
        sd      s6,(-25*8)(sp)
        sd      s7,(-26*8)(sp)
        sd      s8,(-27*8)(sp)
        sd      s9,(-28*8)(sp)
        sd      s10,(-29*8)(sp)
        sd      s11,(-30*8)(sp)
        sd      t3,(-31*8)(sp)
        sd      t4,(-32*8)(sp)
        sd      t5,(-33*8)(sp)
        sd      t6,(-34*8)(sp)

        /* Load kernel gp, tp */
        ld      tp,(-2*8)(sp)
        ld      gp,(-1*8)(sp)

        #if SCHEDULE_BENCHMARK == 1
                /* To allow usage of current->args[] vector */
                sd      a0,(3*8)(tp)
                sd      a1,(4*8)(tp)
                sd      a2,(5*8)(tp)
                sd      a3,(6*8)(tp)
                sd      a4,(7*8)(tp)
                sd      a5,(8*8)(tp)
                sd      a6,(9*8)(tp)
                sd      a7,(10*8)(tp)
        #endif

        /* Store PC */
        csrr    t1,mepc
        sd      t1,(11*8)(tp)

        /* Store SP */
        csrr    t0,mscratch
        sd      t0,(-5*8)(sp)
        
        /* Store mstatus */
        csrr    t0,mstatus
        sd      t0,(-3*8)(sp)


        /* Swap core and process sp */
        amoswap.d  sp,sp,(tp)

        /* Call scheduler */
        call    Sched
        
AsmResumeProc:
        /* Swap core and process sp */
        amoswap.d  sp,sp,(tp)

        csrw    mscratch,sp

        /* Load mstatus */
        ld      t0,(-3*8)(sp)
        csrw    mstatus,t0

        beqz    t0,1f
        /* MPP = 1 */
        sd      sp,(-5*8)(sp)
        csrw    mscratch,zero
1:
        
        /* Load PC */
        ld      t1,(11*8)(tp)
        csrw    mepc,t1

        /* Store kernel gp, tp */
        sd      gp,(-1*8)(sp)
        sd      tp,(-2*8)(sp)


        ld      t6,(-34*8)(sp)
        ld      t5,(-33*8)(sp)
        ld      t4,(-32*8)(sp)
        ld      t3,(-31*8)(sp)
        ld      s11,(-30*8)(sp)
        ld      s10,(-29*8)(sp)
        ld      s9,(-28*8)(sp)
        ld      s8,(-27*8)(sp)
        ld      s7,(-26*8)(sp)
        ld      s6,(-25*8)(sp)
        ld      s5,(-24*8)(sp)
        ld      s4,(-23*8)(sp)
        ld      s3,(-22*8)(sp)
        ld      s2,(-21*8)(sp)
        ld      a7,(-20*8)(sp)
        ld      a6,(-19*8)(sp)
        ld      a5,(-18*8)(sp)
        ld      a4,(-17*8)(sp)
        ld      a3,(-16*8)(sp)
        ld      a2,(-15*8)(sp)
        ld      a1,(-14*8)(sp)
        ld      a0,(-13*8)(sp)
        ld      s1,(-12*8)(sp)
        ld      s0,(-11*8)(sp)
        ld      t2,(-10*8)(sp)
        ld      t1,(-9*8)(sp)
        ld      t0,(-8*8)(sp)

        #if SCHEDULE_BENCHMARK == 1
                /* To allow usage of current->args[] vector */
                ld      a7,(10*8)(tp)
                ld      a6,(9*8)(tp)
                ld      a5,(8*8)(tp)
                ld      a4,(7*8)(tp)
                ld      a3,(6*8)(tp)
                ld      a2,(5*8)(tp)
                ld      a1,(4*8)(tp)
                ld      a0,(3*8)(tp)
        #endif

        ld      tp,(-7*8)(sp)
        ld      gp,(-6*8)(sp)
        ld      ra,(-4*8)(sp)
        ld      sp,(-5*8)(sp)
        mret
