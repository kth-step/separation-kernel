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
        csrr    s0,mstatus

        /* Load kernel gp and tp */
        ld      gp,(-1*8)(sp)
        ld      tp,(-2*8)(sp)

        /* CALL EXCEPTION_HANDLER */
        slli    t0,a1,2
        la      ra,__exception_trampoline
        add     ra,ra,t0
        jalr    ra,(ra)

        /* Load kernel gp and tp */
        sd      gp,(-1*8)(sp)
        sd      tp,(-2*8)(sp)
        
        csrw    mscratch,sp
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

__timer_handler:
        csrrw   sp,mscratch,sp
        bnez    sp,1f
        csrr    sp,mscratch
        addi    sp,sp,-32*8
        sd      gp,(-1*8)(sp) 
        sd      tp,(-2*8)(sp) 
1:      
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
        ld      tp,(-2*8)(sp)
        csrr    a0,mstatus
        sd      a0,(-3*8)(sp)

        amoswap.d sp,sp,(tp)

        call    Sched
trap_resume_proc:

        amoswap.d sp,sp,(tp)
        
        /* Load kernel gp and tp */
        sd      gp,(-1*8)(sp)
        sd      tp,(-2*8)(sp)
        ld      t0,(-3*8)(sp)
        csrw    mstatus,t0

        /* Load pc */
        ld      t1,(0*8)(sp)
        csrw    mepc,t1

        /* save kernel sp */
        csrw    mscratch,sp

        li      t1,0x800
        and     t0,t0,t1
        beqz    t0,1f
        /* If previous privilege was M-mode */
        csrw    mscratch,zero
1:
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

__hang: 
        j       __hang
__mhang: 
        j       __mhang

