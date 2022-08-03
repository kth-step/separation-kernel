# See LICENSE file for copyright and license details.

.globl trap_entry
.globl trap_resume_proc
.globl trap_yield

#include "config.h"
#include "macros.S"


.section .text.trap_entry
.align 8
trap_entry:
        csrrw   tp,mscratch,tp
        bnez    tp,user_trap

machine_trap:
        csrr    a1,mcause
        csrrw   tp,mscratch,zero
#ifndef NDEBUG
        /* Check if we have exception in the kernel, hang */
        bgez    a1,mhang
#endif
        j       interrupt

user_trap:
        /* Save all registers except tp */
        save_context

        /* Save tp */
        csrrw   t0,mscratch,zero
        sd      t0,(4*8)(tp)
        /* Load kernel gp */
        load_gp
        /* Load kernel sp */
        load_sp

        csrr    a1,mcause
        bltz    a1,interrupt

exception:
        /* Enable preemption */
        csrw    mstatus,8
        la      ra,exception_trampoline
        mv      a0,tp
        csrr    a2,mtval
        j       trampoline

interrupt:
        la      ra,interrupt_trampoline

trampoline:
        /* Add trap code offset */
        slli    t0,a1,2
        add     ra,ra,t0
        /* Jump to handler */
        jalr    ra,(ra)
        /* Enable preemption */
        csrw    mstatus,8

trap_resume_proc:
        /* Disable preemption */
        csrw    mstatus,0
        
        /* Set mscratch to proc tp */
        ld      t0,(4*8)(tp)
        csrw    mscratch,t0

        /* Restore all registers except tp */
        restore_context

        /* Swap proc tp with pcb */
        csrrw   tp,mscratch,tp

        mret

trap_yield:
        la      ra,trap_resume_proc
        tail    sched

.option push
.option norvc   /* Prevent compressed instructions */
trap_trampoline:
exception_trampoline:
        /* EXCEPTION HANDLERS */
        j       exception_handler  /* 0: Instruction address misaligned */
        j       exception_handler  /* 1: Instruction access fault */
        j       illegal_instruction_handler  /* 2: Illegal instruction */
        j       exception_handler  /* 3: Breakpoint */
        j       exception_handler  /* 4: Load address misaligned */
        j       exception_handler  /* 5: Load access fault */
        j       exception_handler  /* 6: Store/AMO address misaligned */
        j       exception_handler  /* 7: Store/AMO access fault */
        j       syscall_handler  /* 8: Environment call from U-mode */
        j       hang  /* 9: Environment call from S-mode */
        j       hang  /* 10: Reserved */
        j       hang  /* 11: Environment call from M-mode */
        j       hang  /* 12: Instruction page fault */
        j       hang  /* 13: Load page fault */
        j       hang  /* 14: Reserved */
        j       hang  /* 15: Store/AMO page fault */
interrupt_trampoline:
        /* INTERRUPT HANDLERS */
        j       hang  /*  0: User software interrupt */
        j       hang  /*  1: Supervisor software interrupt */
        j       hang  /*  2: Reserved */
        j       hang  /*  3: Machine software interrupt */
        j       hang  /*  4: User timer interrupt */
        j       hang  /*  5: Supervisor timer interrupt */
        j       hang  /*  6: Reserved */
        j       sched   /*  7: Machine timer interrupt */
        j       hang  /*  8: User external interrupt */
        j       hang  /*  9: Supervisor external interrupt */
        j       hang  /* 10: Reserved */
        j       hang  /* 11: Machine external interrupt */
        j       hang  /* 12: ? */
        j       hang  /* 13: ? */
        j       hang  /* 14: ? */
        j       hang  /* 15: ? */
.option pop

hang:
        j       hang

mhang:
        j       mhang
