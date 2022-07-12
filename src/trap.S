# See LICENSE file for copyright and license details.

.globl trap_entry
.globl trap_resume_proc

#include "config.h"

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
        /* sd      pc,(0*8)(sp) */
        sd      ra,(1*8)(sp)
        /* sd      sp,(2*8)(sp) */
        sd      gp,(3*8)(sp)
        sd      tp,(4*8)(sp)
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

        /* Save pc and sp */
        csrr    t0,mepc
        csrrw   t1,mscratch,zero
        sd      t0,(0*8)(sp)
        sd      t1,(2*8)(sp)


        mv      a0,sp
        csrr    a1,mcause
        csrr    a2,mtval 
        mv      s1,sp

        /* Load kernel gp and tp */
        ld      gp,(-1*8)(sp)
        ld      tp,(-2*8)(sp)
        csrrs   s0,mstatus,8

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
        addi    sp,sp,-32*8
        sd      gp,(-1*8)(sp) 
        sd      tp,(-2*8)(sp) 
        sd      zero,(-4*8)(sp)
        j       2f
1:      
        sd      sp,(-4*8)(sp)
2:
        /* sd      pc,(0*8)(sp) */
        sd      ra,(1*8)(sp)
        /* sd      sp,(2*8)(sp) */
        sd      gp,(3*8)(sp)
        sd      tp,(4*8)(sp)
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

        /* Save pc and sp */
        csrr    t0,mepc
        csrrw   t1,mscratch,zero
        sd      t0,(0*8)(sp)
        sd      t1,(2*8)(sp)

        /* Load kernel gp and tp */
        ld      gp,(-1*8)(sp)
        ld      a0,(-2*8)(sp)
        csrr    t0,mstatus
        sd      t0,(-3*8)(sp)

        amoswap.d sp,sp,(a0)

        addi    t0,a0,(5*8)     /* ptr to state */
        li      t1,4
        amoand.w x0,t1,(t0)     /* zero lower two bits of state */

__sched:
        call    Sched
trap_resume_proc:

        addi    t0,a0,(5*8)
        li      t2,1

        /* CompareAndSwap(state, PROC_READY, PROC_RUNNING */
1:      lr.w    t1,(t0)
        bnez    t1,__sched
        sc.w    t3,t2,(t0)
        bnez    t3,1b

        amoswap.d sp,sp,(a0)
        
        mv      tp,a0
        ld      s0,(-3*8)(sp)
        ld      s1,(-4*8)(sp)
__restore_registers:
        /* Load kernel gp and tp */
        sd      gp,(-1*8)(sp)
        sd      tp,(-2*8)(sp)
        
        csrw    mscratch,s1
        csrw    mstatus,s0
        /* Load pc and sp */
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
        ld      sp,(2*8)(sp)
        mret

trap_recv_yield:
        /* Swap to core stack */
        addi    t0,tp,(5*8)
        li      t2,1
        li      t3,2
        /* CompareAndSwap(state,PROC_RUNNING, PROC_WAITING) */
1:      lr.w    t1,(t0)
        bne     t1,t2,2f
        sc.w    t1,t3,(t0)
        bnez    t1,1b          
        amoswap.d  sp,sp,(tp)
        sd      gp,(-1*8)(sp) 
        sd      tp,(-2*8)(sp) 
        csrr    t0,mstatus
        sd      t0,(-3*8)(sp)
        j       __sched
2:      ret

__hang: 
        j       __hang
__mhang: 
        j       __mhang

