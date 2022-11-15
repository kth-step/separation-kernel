// See LICENSE file for copyright and license details.
#include "macros.S"
#include "asm_consts.h"

#define MCAUSE_U_ECALL 8
#define MSTATUS_IE 8

.globl trap_entry
.globl trap_resume_proc

.section .text.trap
.align 8
trap_entry:
        /* Save tp, load pcb */ 
        csrrw   tp,mscratch,tp

        /* if tp == 0, then M-mode preemption */
        beqz    tp,trap_timer_preempted   

        /* Save all registers except tp and pc */
        save_context

        /* Save tp */
        csrr    s0,mepc
        csrrw   t1,mscratch,zero
        sd      s0,(PROC_REGS + REGS_PC)(tp)
        sd      t1,(PROC_REGS + REGS_TP)(tp)

        /* Load kernel gp and sp */
        load_gp
        load_sp

        /* mcause < 0, must be timer interrupt so yield */
        csrr    s1,mcause
        bltz    s1,trap_timer

        /* if mcause == User ecall, it is a system call */ 
        li      t1,MCAUSE_U_ECALL
        beq     s1,t1,trap_syscall

trap_exception:
        mv      a0,s1
        mv      a2,s0
        csrr    a1,mtval
        call    exception_handler
        j       trap_resume_proc

trap_timer_preempted:
        csrrw   tp,mscratch,zero
        csrr    t0,mcause
        bgez    t0,hang
trap_timer:
        tail    sched_yield

trap_syscall:
        /* Incr. pc with 4 */
        addi    s0,s0,4
        sd      s0,(PROC_REGS + REGS_PC)(tp)

        /* if a7 >= NUM_OF_SYSNR (or a7 == 0), then syscall_unimplemented */
1:      auipc   ra,%pcrel_hi(syscall_vector)
        li      t1,NUM_OF_SYSNR
        bgeu    t0,t1,2f

        /* Jump to syscall handler */
        slli    t0,t0,2
        add     ra,ra,t0
2:      jalr    %pcrel_lo(1b)(ra)
        sd      a0,(PROC_REGS + REGS_A0)(tp)

trap_resume_proc:
        /* Enable preemption */
        csrw    mstatus,MSTATUS_IE

        /* Restore pc */
        ld      t0,(PROC_REGS + REGS_PC)(tp)
        csrw    mepc,t0

        /* Restore all registers except tp, sp, gp, pc */
        restore_context

        /* Disable preemption */
        csrw    mstatus,0
        /* Save pointer to proc_t */
        csrw    mscratch,tp
        /* Restore user sp, gp and tp */
        ld      sp,(PROC_REGS + REGS_SP)(tp)
        ld      gp,(PROC_REGS + REGS_GP)(tp)
        ld      tp,(PROC_REGS + REGS_TP)(tp)
        /* Return to user mode */
        mret

syscall_vector:
.option push
.option norvc
        j       syscall_unimplemented
        j       syscall_read_cap
        j       syscall_move_cap
        j       syscall_delete_cap
        j       syscall_revoke_cap
        j       syscall_derive_cap
        j       syscall_invoke_cap
        j       syscall_get_pid
        j       syscall_read_reg
        j       syscall_write_reg
        j       syscall_yield
.option pop

hang:
        wfi
        j       hang
