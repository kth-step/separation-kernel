# See LICENSE file for copyright and license details.
#include "config.h"

.globl AsmTrapEntry
.globl AsmResumeProc
.globl trap_recv_yield
.globl sys_to_sched

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
        /* Ensure preemption is disabled.
           We assume we can only get here from user code; if we don't then we have 
           encountered an exception in the kernel and we don't support recovery from that. 
           Thus, it's fine if we overwrite the value of mie in mstatus and assume the only other
           situation where it is relevant, timer interrupt, doesn't rely on it being preserved
           between different timer interrupts. */
        csrci   mstatus,8
        /* Load kernel process sp, store user sp. */
        csrrw   sp,mscratch,sp
#ifdef DEBUG
        /* If mscratch is 0, we have bug in the kernel */
        beqz    sp,__mhang
#endif
        sd      ra,(-5*8)(sp)
        sd      gp,(-7*8)(sp)
        sd      tp,(-8*8)(sp)
        sd      t0,(-9*8)(sp)
        sd      t1,(-10*8)(sp)
        sd      t2,(-11*8)(sp)
        sd      s0,(-12*8)(sp)
        sd      s1,(-13*8)(sp)
        sd      a0,(-14*8)(sp)
        sd      a1,(-15*8)(sp)
        sd      a2,(-16*8)(sp)
        sd      a3,(-17*8)(sp)
        sd      a4,(-18*8)(sp)
        sd      a5,(-19*8)(sp)
        sd      a6,(-20*8)(sp)
        sd      a7,(-21*8)(sp)
        sd      s2,(-22*8)(sp)
        sd      s3,(-23*8)(sp)
        sd      s4,(-24*8)(sp)
        sd      s5,(-25*8)(sp)
        sd      s6,(-26*8)(sp)
        sd      s7,(-27*8)(sp)
        sd      s8,(-28*8)(sp)
        sd      s9,(-29*8)(sp)
        sd      s10,(-30*8)(sp)
        sd      s11,(-31*8)(sp)
        sd      t3,(-32*8)(sp)
        sd      t4,(-33*8)(sp)
        sd      t5,(-34*8)(sp)
        sd      t6,(-35*8)(sp)

        /* Save user sp */
        csrrw   t1,mscratch,zero
        sd      t1,(-6*8)(sp)

        /* Store mstatus */
        csrr    t0,mstatus
        sd      t0,(-2*8)(sp)
        /* Save mstatus */
        #csrrs   t0,mstatus,8
        #sd      t0,(-2*8)(sp)

        /* Load kernel gp and tp */
        ld      gp,(-3*8)(sp) /* points to kernel global data */
        ld      tp,(-1*8)(sp) /* points to current pcb */
        
        /* Save user a0-a7 in proc struct. */
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
        sd      t3,(-4*8)(sp)

        /* To be able to restore sp if going directly to scheduling from syscall */
        sd      sp,(12*8)(tp)

        /* Fix sp and enable preemption */
        addi    sp,sp,-36*8
        csrsi   mstatus,8

        call    SyscallHandler

        /* Fix sp and disable preemption */
        csrci   mstatus,8
        addi    sp,sp,36*8

        j       __exception_exit

__exception:
        j       __exception_handler_unimplemented_hang
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
        sd      t3,(-4*8)(sp)

        addi    sp,sp,-36*8
        csrsi   mstatus,8

        /* Jump to trampoline */
        add     ra,ra,t1
        jalr    ra,0(ra)

        /* Fix sp and disable preemption */
        csrci   mstatus,8
        addi    sp,sp,36*8
        j       __exception_exit

__hang: 
        j       __hang
__exception_handler_unimplemented_hang: 
        j       __exception_handler_unimplemented_hang
__mhang: 
        j       __mhang

__timer_handler:
        csrrw   sp,mscratch,sp
        bnez    sp,1f
__timer_handler_machine:
        csrr    sp,mscratch
        addi    sp,sp,-36*8
        /* Store kernel gp, tp */
        sd      gp,(-3*8)(sp)
        sd      tp,(-1*8)(sp)

        sd      zero,(-6*8)(sp)
        sd      t0,(-9*8)(sp)
        j       2f
1:
        /* Store user sp, but first store t0 so we can use it */
        sd      t0,(-9*8)(sp)
        csrr    t0,mscratch
        sd      t0,(-6*8)(sp)
2:      
        sd      ra,(-5*8)(sp)
        sd      gp,(-7*8)(sp)
        sd      tp,(-8*8)(sp)
        
        sd      t1,(-10*8)(sp)
        sd      t2,(-11*8)(sp)
        sd      s0,(-12*8)(sp)
        sd      s1,(-13*8)(sp)
        sd      a0,(-14*8)(sp)
        sd      a1,(-15*8)(sp)
        sd      a2,(-16*8)(sp)
        sd      a3,(-17*8)(sp)
        sd      a4,(-18*8)(sp)
        sd      a5,(-19*8)(sp)
        sd      a6,(-20*8)(sp)
        sd      a7,(-21*8)(sp)
        sd      s2,(-22*8)(sp)
        sd      s3,(-23*8)(sp)
        sd      s4,(-24*8)(sp)
        sd      s5,(-25*8)(sp)
        sd      s6,(-26*8)(sp)
        sd      s7,(-27*8)(sp)
        sd      s8,(-28*8)(sp)
        sd      s9,(-29*8)(sp)
        sd      s10,(-30*8)(sp)
        sd      s11,(-31*8)(sp)
        sd      t3,(-32*8)(sp)
        sd      t4,(-33*8)(sp)
        sd      t5,(-34*8)(sp)
        sd      t6,(-35*8)(sp)

        /* Save sp */
        //csrr    t1,mscratch
        //sd      t1,(-6*8)(sp)

        /* Store mstatus */
        csrr    t0,mstatus
        sd      t0,(-2*8)(sp)

        /* Load kernel gp and tp */
        ld      gp,(-3*8)(sp) /* points to kernel global data */
        ld      tp,(-1*8)(sp) /* points to current pcb */

        #if SCHEDULE_BENCHMARK != 0
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
        //sd      t1,(11*8)(tp)
        sd      t1,(-4*8)(sp)

        /* Swap core and process sp */
        amoswap.d  sp,sp,(tp)

        /* Call scheduler */
        call    Sched
        
AsmResumeProc:
        /* Swap core and process sp */
        amoswap.d  sp,sp,(tp)
        ld      a7,(-21*8)(sp)
        ld      a6,(-20*8)(sp)
        ld      a5,(-19*8)(sp)
        ld      a4,(-18*8)(sp)
        ld      a3,(-17*8)(sp)
        ld      a2,(-16*8)(sp)
        ld      a1,(-15*8)(sp)
        ld      a0,(-14*8)(sp)
        #if SCHEDULE_BENCHMARK == 0 // In schedule benchmark we want to use the args from current->args
        j       3f
        #endif
__exception_exit:
        /* To allow usage of current->args[] vector */
        ld      a7,(10*8)(tp)
        ld      a6,(9*8)(tp)
        ld      a5,(8*8)(tp)
        ld      a4,(7*8)(tp)
        ld      a3,(6*8)(tp)
        ld      a2,(5*8)(tp)
        ld      a1,(4*8)(tp)
        ld      a0,(3*8)(tp)

3:
        /* Load PC */
        //ld      t1,(11*8)(tp)
        ld      t1,(-4*8)(sp)
        csrw    mepc,t1

        /* Store kernel gp, tp */
        sd      gp,(-3*8)(sp)
        sd      tp,(-1*8)(sp)

        /* Load mstatus */
        ld      t0,(-2*8)(sp)
        /* In exception handeling we have ensured that preemption is disabled in the stored mstatus.
           If preemption is enabled in mstatus it must have been stored from a timer interrupt,
           meaning we are returning from such, and it's not possible for a timer interrupt to be
           interrupted by another timer interrupt. Thus, it's fine to just load mstatus, even though it might 
           enable preemption already. */
        csrw    mstatus,t0

        ld      t0,(-6*8)(sp)
        csrw    mscratch,t0

        ld      t6,(-35*8)(sp)
        ld      t5,(-34*8)(sp)
        ld      t4,(-33*8)(sp)
        ld      t3,(-32*8)(sp)
        ld      s11,(-31*8)(sp)
        ld      s10,(-30*8)(sp)
        ld      s9,(-29*8)(sp)
        ld      s8,(-28*8)(sp)
        ld      s7,(-27*8)(sp)
        ld      s6,(-26*8)(sp)
        ld      s5,(-25*8)(sp)
        ld      s4,(-24*8)(sp)
        ld      s3,(-23*8)(sp)
        ld      s2,(-22*8)(sp)

        ld      s1,(-13*8)(sp)
        ld      s0,(-12*8)(sp)
        ld      t2,(-11*8)(sp)
        ld      t1,(-10*8)(sp)
        ld      t0,(-9*8)(sp)

        /* Store kernel gp and tp. */
        sd      tp,(-1*8)(sp)
        sd      gp,(-3*8)(sp)

        ld      tp,(-8*8)(sp)
        ld      gp,(-7*8)(sp)
        ld      ra,(-5*8)(sp)

        csrrw   sp,mscratch,sp
        bnez    sp,4f
        csrr    sp,mscratch
        addi    sp,sp,36*8
        csrw    mscratch,zero
4:
        mret

sys_to_sched:
        /* Fix sp and disable preemption */
        // TODO: do we want to disable preemption here or after sched?
        csrci   mstatus,8
        ld      sp,(12*8)(tp)
        /* Make sure the syscall returns true since it doesn't return normally */
        li      a0,1
        sd      a0,(3*8)(tp)
        sd      a0,(-14*8)(sp)
        /* Swap to core stack */
        amoswap.d  sp,sp,(tp)
        call    Sched
        amoswap.d  sp,sp,(tp)
        j       __exception_exit

trap_recv_yield:
        /* CompareAndSwap(state,PROC_RUNNING, PROC_WAITING) */
        addi    t1,tp,(5*8)
        li      t2,1
        li      t3,2
        casw    t0,t1,t2,t3,__recv_fail
        
        /* Reschedule */
        j       sys_to_sched 
__recv_fail:  /* CAS fail */
        ret