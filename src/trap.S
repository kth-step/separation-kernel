# See LICENSE file for copyright and license details.

.globl trap_entry
.globl trap_resume_proc
.globl trap_recv_yield

#include "config.h"

/* tmp -> working register
 * addr -> addr to lr/sc
 * expected -> register with expected value
 * desired -> register with desired value
 * failure_label -> where to jump of CAS failure
 */
.macro casw tmp,addr,expected,desired,failure_label
42:     lr.w    \tmp,(\addr)
        bne     \tmp,\expected,\failure_label
        sc.w    \tmp,\desired,(\addr)
        bnez    \tmp,42b
.endm

.option push
.option norvc   /* Prevent compressed instructions */
.align 8
trap_entry:
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
        j       ExceptionHandler  /* 0: Instruction address misaligned */
        j       ExceptionHandler  /* 1: Instruction access fault */
        j       ExceptionHandler  /* 2: Illegal instruction */
        j       ExceptionHandler  /* 3: Breakpoint */
        j       ExceptionHandler  /* 4: Load address misaligned */
        j       ExceptionHandler  /* 5: Load access fault */
        j       ExceptionHandler  /* 6: Store/AMO address misaligned */
        j       ExceptionHandler  /* 7: Store/AMO access fault */
        j       SyscallHandler  /* 8: Environment call from U-mode */
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
#ifndef NDEBUG
        /* If mscratch is 0, we have bug in the kernel */
        beqz    sp,__mhang
#endif
        sd      ra,(5*8)(sp)
        sd      gp,(7*8)(sp)
        sd      tp,(8*8)(sp)
        sd      t0,(9*8)(sp)
        sd      t1,(10*8)(sp)
        sd      t2,(11*8)(sp)
        sd      s0,(12*8)(sp)
        sd      s1,(13*8)(sp)
        sd      a0,(14*8)(sp)
        sd      a1,(15*8)(sp)
        sd      a2,(16*8)(sp)
        sd      a3,(17*8)(sp)
        sd      a4,(18*8)(sp)
        sd      a5,(19*8)(sp)
        sd      a6,(20*8)(sp)
        sd      a7,(21*8)(sp)
        sd      s2,(22*8)(sp)
        sd      s3,(23*8)(sp)
        sd      s4,(24*8)(sp)
        sd      s5,(25*8)(sp)
        sd      s6,(26*8)(sp)
        sd      s7,(27*8)(sp)
        sd      s8,(28*8)(sp)
        sd      s9,(29*8)(sp)
        sd      s10,(30*8)(sp)
        sd      s11,(31*8)(sp)
        sd      t3,(32*8)(sp)
        sd      t4,(33*8)(sp)
        sd      t5,(34*8)(sp)
        sd      t6,(35*8)(sp)

        /* Save pc and sp */
        csrr    t0,mepc
        csrrw   t1,mscratch,zero
        sd      t0,(4*8)(sp)
        sd      t1,(6*8)(sp)


        mv      a0,sp
        csrr    a1,mcause
        csrr    a2,mtval 

        /* Load kernel gp and tp */
        ld      gp,(0*8)(sp)
        ld      tp,(1*8)(sp)
        /* Save mstatus */
        csrrs   t0,mstatus,8
        sd      t0,(2*8)(sp)

        /* CALL EXCEPTION_HANDLER */
        slli    t0,a1,2
        la      t1,__exception_trampoline
        add     t1,t1,t0
        la      ra,__restore_registers
        jalr    x0,(t1)



__timer_handler:
        csrrw   sp,mscratch,sp
        bnez    sp,1f
        csrr    sp,mscratch
        addi    sp,sp,-36*8
        sd      gp,(0*8)(sp) 
        sd      tp,(1*8)(sp) 
        sd      zero,(3*8)(sp)
1:
        sd      ra,(5*8)(sp)
        sd      gp,(7*8)(sp)
        sd      tp,(8*8)(sp)
        sd      t0,(9*8)(sp)
        sd      t1,(10*8)(sp)
        sd      t2,(11*8)(sp)
        sd      s0,(12*8)(sp)
        sd      s1,(13*8)(sp)
        sd      a0,(14*8)(sp)
        sd      a1,(15*8)(sp)
        sd      a2,(16*8)(sp)
        sd      a3,(17*8)(sp)
        sd      a4,(18*8)(sp)
        sd      a5,(19*8)(sp)
        sd      a6,(20*8)(sp)
        sd      a7,(21*8)(sp)
        sd      s2,(22*8)(sp)
        sd      s3,(23*8)(sp)
        sd      s4,(24*8)(sp)
        sd      s5,(25*8)(sp)
        sd      s6,(26*8)(sp)
        sd      s7,(27*8)(sp)
        sd      s8,(28*8)(sp)
        sd      s9,(29*8)(sp)
        sd      s10,(30*8)(sp)
        sd      s11,(31*8)(sp)
        sd      t3,(32*8)(sp)
        sd      t4,(33*8)(sp)
        sd      t5,(34*8)(sp)
        sd      t6,(35*8)(sp)

        /* Save pc and sp */
        csrr    t0,mepc
        csrrw   t1,mscratch,zero
        sd      t0,(4*8)(sp)
        sd      t1,(6*8)(sp)

        /* Load kernel gp and tp */
        ld      gp,(0*8)(sp)
        ld      tp,(1*8)(sp)
        /* Save mstatus */
        csrr    t0,mstatus
        sd      t0,(2*8)(sp)

        amoswap.d sp,sp,(tp)

        /* Set the state to PROC_READY/PROC_SUSPENDED */
        addi    t0,tp,(5*8)     /* ptr to state */
        li      t1,4
        amoand.w x0,t1,(t0)     /* zero lower two bits of state */

__sched:
        call    Sched
trap_resume_proc:
        /* CompareAndSwap(state, PROC_READY, PROC_RUNNING) */
        addi    t1,tp,(5*8)
        li      t2,1
        casw    t0,t1,x0,t2,__sched

        amoswap.d sp,sp,(tp)
        
__restore_registers:

        ld      t0,(2*8)(sp)
        csrw    mstatus,t0
        ld      t0,(3*8)(sp)
        csrw    mscratch,t0
        ld      t0,(4*8)(sp)
        csrw    mepc,t0

        ld      t6,(35*8)(sp)
        ld      t5,(34*8)(sp)
        ld      t4,(33*8)(sp)
        ld      t3,(32*8)(sp)
        ld      s11,(31*8)(sp)
        ld      s10,(30*8)(sp)
        ld      s9,(29*8)(sp)
        ld      s8,(28*8)(sp)
        ld      s7,(27*8)(sp)
        ld      s6,(26*8)(sp)
        ld      s5,(25*8)(sp)
        ld      s4,(24*8)(sp)
        ld      s3,(23*8)(sp)
        ld      s2,(22*8)(sp)
        ld      a7,(21*8)(sp)
        ld      a6,(20*8)(sp)
        ld      a5,(19*8)(sp)
        ld      a4,(18*8)(sp)
        ld      a3,(17*8)(sp)
        ld      a2,(16*8)(sp)
        ld      a1,(15*8)(sp)
        ld      a0,(14*8)(sp)
        ld      s1,(13*8)(sp)
        ld      s0,(12*8)(sp)
        ld      t2,(11*8)(sp)
        ld      t1,(10*8)(sp)
        ld      t0,(9*8)(sp)
        ld      tp,(8*8)(sp)
        ld      gp,(7*8)(sp)
        ld      ra,(5*8)(sp)
        ld      sp,(6*8)(sp)
        mret

trap_recv_yield:
        /* CompareAndSwap(state,PROC_RUNNING, PROC_WAITING) */
        addi    t1,tp,(5*8)
        li      t2,1
        li      t3,2
        casw    t0,t1,t2,t3,__recv_fail
        /* Swap to core stack */
        amoswap.d  sp,sp,(tp)
        /* Reschedule */
        j       __sched
__recv_fail:  /* CAS fail */
        ret

__hang: 
        j       __hang
__mhang: 
        j       __mhang

