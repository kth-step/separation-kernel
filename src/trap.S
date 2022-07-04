# See LICENSE file for copyright and license details.

.globl trap_entry
.globl trap_resume_proc

.option push
.option norvc   /* Prevent compressed instructions */
.align 8
trap_entry:
        j       __trap_exception_handler  /*  0: User software interrupt */
        /* Interrupts */
        j       __hang  /*  1: Supervisor software interrupt */
        j       __hang  /*  2: Reserved */
        j       __hang  /*  3: Machine software interrupt */
        j       __hang  /*  4: User timer interrupt */
        j       __hang  /*  5: Supervisor timer interrupt */
        j       __hang  /*  6: Reserved */
        j       __trap_timer_handler /*  7: Machine timer interrupt */
        j       __hang  /*  8: User external interrupt */
        j       __hang  /*  9: Supervisor external interrupt */
        j       __hang  /* 10: Reserved */
        j       __hang  /* 11: Machine external interrupt */

__trap_exception_trampoline:
        /* Exceptions */
        j       __hang  /* 0: Instruction address misaligned */
        j       __hang  /* 1: Instruction access fault */
        j       __hang  /* 2: Illegal instruction */
        j       __hang  /* 3: Breakpoint */
        j       __hang  /* 4: Load address misaligned */
        j       __hang  /* 5: Load access fault */
        j       __hang  /* 6: Store/AMO address misaligned */
        j       __hang  /* 7: Store/AMO access fault */
        j       syscall_handler  /* 8: Environment call from U-mode */
        j       __hang  /* 9: Environment call from S-mode */
        j       __hang  /* 10: Reserved */
        j       __hang  /* 11: Environment call from M-mode */
        j       __hang  /* 12: Instruction page fault */
        j       __hang  /* 13: Load page fault */
        j       __hang  /* 14: Reserved */
        j       __hang  /* 15: Store/AMO page fault */
.option pop

/****** Exception and system call *******/
__trap_exception_handler:
        /* Load kernel process sp, store user sp. */
        csrrw   sp,mscratch,sp
#ifndef NDEBUG
        /* If mscratch is 0, we have bug in the kernel */
        beqz    sp,__mhang
#endif
        addi    sp,sp,(-24*8)
        /* Store ra to stack */
        sd      ra,(1*8)(sp)

        /* Store pc and sp to trap_frame */
        csrr    ra,mepc
        sd      ra,(0*8)(sp)
        csrrw   ra,mscratch,zero
        sd      ra,(2*8)(sp)

        /* Store gp, tp to trap_frame */
        sd      gp,(3*8)(sp)
        sd      tp,(4*8)(sp)

        /* Store mtval and mcause to trap_frame */
        csrr    ra,mtval
        sd      ra,(21*8)(sp)
        csrr    ra,mcause               /* note, ra=mcause will be used later */
        sd      ra,(20*8)(sp)

        /* Load kernel gp and thread pcb */
        ld      gp,(22*8)(sp)
        ld      tp,(23*8)(sp)

        /* Enable preemption */
        /* Before enabling preemption, we must store mcause, mtval, mepc, mscratch,
         * and load kernel gp and thread pcb (tp).
         */
        csrsi   mstatus,8

        /* Store argument temporary registers to trap_frame */
        sd      t0,(5*8)(sp)
        sd      t1,(6*8)(sp)
        sd      t2,(7*8)(sp)
        sd      t3,(8*8)(sp)
        sd      t4,(9*8)(sp)
        sd      t5,(10*8)(sp)
        sd      t6,(11*8)(sp)

        sd      a0,(12*8)(sp)
        sd      a1,(13*8)(sp)
        sd      a2,(14*8)(sp)
        sd      a3,(15*8)(sp)
        sd      a4,(16*8)(sp)
        sd      a5,(17*8)(sp)
        sd      a6,(18*8)(sp)
        sd      a7,(19*8)(sp)

        /* sp->trap_frame */
        mv      a0,sp

        /* Jump to trampoline */
        slli    t0,ra,2         /* ra=mcause */
        la      ra,__trap_exception_trampoline
        add     ra,ra,t0
        jalr    ra,0(ra)

__trap_exception_exit:

        /* Load argument and temporary registers */
        ld      a7,(19*8)(sp)
        ld      a6,(18*8)(sp)
        ld      a5,(17*8)(sp)
        ld      a4,(16*8)(sp)
        ld      a3,(15*8)(sp)
        ld      a2,(14*8)(sp)
        ld      a1,(13*8)(sp)
        ld      a0,(12*8)(sp)

        ld      t6,(11*8)(sp)
        ld      t5,(10*8)(sp)
        ld      t4,(9*8)(sp)
        ld      t3,(8*8)(sp)
        ld      t2,(7*8)(sp)
        ld      t1,(6*8)(sp)
        ld      t0,(5*8)(sp)

        /* Enable preemption */
        csrsi   mstatus,8

        /* Load user gp,tp */
        ld      tp,(4*8)(sp)
        ld      gp,(3*8)(sp)

        /* Load user pc and sp */
        ld      ra,(2*8)(sp)
        csrw    mscratch,ra
        ld      ra,(0*8)(sp)
        csrw    mepc,ra

        /* Load user ra */
        ld      ra,(1*8)(sp)

        /* Restore kernel sp */
        addi    sp,sp,(24*8)

        /* Swap user sp and kernel sp */
        csrrw   sp,mscratch,sp 
        mret

/****** Timer interrupt *******/
__trap_timer_handler:
        csrrw   sp,mscratch,sp
        bnez    sp,__trap_timer_handler_user
__trap_timer_handler_machine:
        csrr    sp,mscratch
        sd      gp,(-2*8)(sp)
        sd      tp,(-1*8)(sp)
__trap_timer_handler_user: 
        sd      ra,(-34*8)(sp)
        csrrw   ra,mscratch,zero
        sd      ra,(-33*8)(sp)
        sd      gp,(-32*8)(sp)
        sd      tp,(-31*8)(sp)
        sd      t0,(-30*8)(sp)
        sd      t1,(-29*8)(sp)
        sd      t2,(-28*8)(sp)
        sd      t3,(-27*8)(sp)
        sd      t4,(-26*8)(sp)
        sd      t5,(-25*8)(sp)
        sd      t6,(-24*8)(sp)
        sd      a0,(-23*8)(sp)
        sd      a1,(-22*8)(sp)
        sd      a2,(-21*8)(sp)
        sd      a3,(-20*8)(sp)
        sd      a4,(-19*8)(sp)
        sd      a5,(-18*8)(sp)
        sd      a6,(-17*8)(sp)
        sd      a7,(-16*8)(sp)
        sd      s0,(-15*8)(sp)
        sd      s1,(-14*8)(sp)
        sd      s2,(-13*8)(sp)
        sd      s3,(-12*8)(sp)
        sd      s4,(-11*8)(sp)
        sd      s5,(-10*8)(sp)
        sd      s6,(-9*8)(sp)
        sd      s7,(-8*8)(sp)
        sd      s8,(-7*8)(sp)
        sd      s9,(-6*8)(sp)
        sd      s10,(-5*8)(sp)
        sd      s11,(-4*8)(sp)

        /* Store mstatus */
        csrr    t0,mstatus
        sd      t0,(-3*8)(sp)

        ld      gp,(-2*8)(sp)
        ld      tp,(-1*8)(sp)

        /* Store PC */
        csrr    t1,mepc
        sd      t1,(2*8)(tp)
        


        addi    sp,sp,(-34*8)
        /* Swap core and process sp */
        amoswap.d  sp,sp,(tp)

        /* Call scheduler */
        call    Sched
        
trap_resume_proc:
        /* Swap core and process sp */
        amoswap.d  sp,sp,(tp)

        addi    sp,sp,(34*8)


        /* Load PC */
        ld      t1,(2*8)(tp)
        csrw    mepc,t1

        /* Store kernel gp and pcb */
        sd      gp,(-2*8)(sp)
        sd      tp,(-1*8)(sp)

        /* Load mstatus */
        ld      t0,(-3*8)(sp)
        csrw    mstatus,t0

        /* Mask bits so we get mstatus.mpp */
        li      t1,0x800
        and     t0,t0,t1

        beqz    t0,1f   /* jump if 1 or 3 (supervisor or user) */
        /* mstatus.mpp == 0 (m-mode) */
        sd      sp,(-33*8)(sp) 

1:      ld      s11,(-4*8)(sp)
        ld      s10,(-5*8)(sp)
        ld      s9,(-6*8)(sp)
        ld      s8,(-7*8)(sp)
        ld      s7,(-8*8)(sp)
        ld      s6,(-9*8)(sp)
        ld      s5,(-10*8)(sp)
        ld      s4,(-11*8)(sp)
        ld      s3,(-12*8)(sp)
        ld      s2,(-13*8)(sp)
        ld      s1,(-14*8)(sp)
        ld      s0,(-15*8)(sp)
        ld      a7,(-16*8)(sp)
        ld      a6,(-17*8)(sp)
        ld      a5,(-18*8)(sp)
        ld      a4,(-19*8)(sp)
        ld      a3,(-20*8)(sp)
        ld      a2,(-21*8)(sp)
        ld      a1,(-22*8)(sp)
        ld      a0,(-23*8)(sp)
        ld      t6,(-24*8)(sp)
        ld      t5,(-25*8)(sp)
        ld      t4,(-26*8)(sp)
        ld      t3,(-27*8)(sp)
        ld      t2,(-28*8)(sp)
        ld      t1,(-29*8)(sp)
        ld      t0,(-30*8)(sp)
        ld      tp,(-31*8)(sp)
        ld      gp,(-32*8)(sp)
        ld      ra,(-33*8)(sp)
        csrw    mscratch,ra
        ld      ra,(-34*8)(sp)
        csrrw   sp,mscratch,sp
        mret

__hang: 
        j       __hang
__mhang: 
        j       __mhang

