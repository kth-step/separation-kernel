# See LICENSE file for copyright and license details.

.globl trap_entry
.globl trap_resume_proc
.globl trap_recv_yield

#include "config.h"
#include "macros.S"


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
        csrrw   tp,mscratch,tp
#ifndef NDEBUG
        /* If mscratch is 0, we have bug in the kernel */
        beqz    tp,__mhang
#endif
        
        /* Save all registers except tp */
        save_context

        /* Save pc */
        csrrw   t0,mscratch,zero
        sd      t0,(4*8)(tp)

        load_gp_sp

        /* Enable preemption */
        csrw    mstatus,8

        /* CALL EXCEPTION_HANDLER */
        mv      a0,tp
        csrr    a1,mcause
        csrr    a2,mtval

        slli    t0,a1,2
        la      t1,__exception_trampoline
        add     t1,t1,t0
        la      ra,trap_resume_proc
        jalr    x0,(t1)

__timer_handler:
        csrrw   tp,mscratch,tp
        bnez    tp,1f
        csrrw   tp,mscratch,zero
        j       2f
1:
        /* Save all registers except pc/tp */
        save_context

        /* Save tp */
        csrr    t0,mscratch
        sd      t0,(4*8)(tp)

        /* Load kernel gp and sp */
        load_gp_sp
2:

        call sched

trap_resume_proc:

        /* Disable preemption */
        csrw    mstatus,0
        
        /* Set mscratch to proc tp */
        ld      t0,(4*8)(tp)
        csrw    mscratch,t0

        /* Restore all registers except pc and tp */
        restore_context
        /* Swap proc tp with pcb */
        csrrw   tp,mscratch,tp

        mret

__hang: 
        j       __hang
__mhang: 
        j       __mhang

