# See LICENSE file for copyright and license details.
#include "s3k_consts.h"
#include "offsets.g.h"
#include "config.h"
#include "macros.S"

#define MCAUSE_U_ECALL 8
#define MSTATUS_IE 8

.globl trap_entry
.globl trap_syscall_exit1
.globl trap_syscall_exit2
.globl trap_resume_proc

.section .text.trap
.align 8
trap_entry:
        /* Save tp, load pcb */ 
        csrrw   tp,mscratch,tp
        bnez    tp,_trap_umode   /* if tp != 0, then U-mode trap */
_trap_mmode:
        csrrw   tp,mscratch,zero /* Restore pcb */
        j       sched_yield     /* Yield */
_trap_umode:
        /* Save all registers except tp */
        save_context

        /* Save tp */
        csrrw   t0,mscratch,zero
        sd      t0,(PROC_REGS + REGS_TP)(tp)

        /* Load kernel gp */
        load_gp
        /* Load kernel sp */
        load_sp

        /* Get the trap cause */
        csrr    t0,mcause
        
        /* mcause < 0, must be timer interrupt so yield */
        bltz    t0,sched_yield

        /* Enable preemption */
        csrw    mstatus,MSTATUS_IE

        la      s0,exception_handler
        /* Check if ecall from U mode */ 
        li      t1,MCAUSE_U_ECALL
        bne     t0,t1,1f

trap_syscall:
        la      s0,syscall_unimplemented
        li      t0,NUM_OF_SYSNR
        bgeu    a7,t0,2f        /* a7 >= NUM_OF_SYSNR, then syscall_unimplemented */
        li      t0,S3K_SYSNR_INVOKE_CAP
        bltu    t0,a7,1f        /* a7 >= SYSNR_INVOKE_CAP, then dont fetch capability */

        /* Fetch capability */
        andi    a0,a0,(N_CAPS - 1) /* a0 = a0 % N_CAPS */
        slli    a0,a0,5         /* a0 = a0 * 32 (width of cap_node) */
        ld      t0,(PROC_CAP_TABLE)(tp)
        add     a0,a0,t0
        ld      a1,(CAP_NODE_CAP + CAP_WORD0)(a0)
        ld      a2,(CAP_NODE_CAP + CAP_WORD1)(a0)
        fence
        ld      t0,CAP_NODE_PREV(a0)
        beqz    t0,2f   /* if cap_node is deleted, then syscall unimplemented */

        /* Jump to syscall handler */
1:      la      s0,_syscall_vector
        slli    t0,a7,2
        add     s0,s0,t0
2:      jalr    s0

/* Return syscall with error code in a0, and turning off interrupts */
trap_syscall_exit1:
        csrw    mstatus,0

/* Return syscall with error code in a0, and not turning off interrupts */
trap_syscall_exit2:
        sd      a0,(PROC_REGS + REGS_A0)(tp)    /* Save the syscall code */
        ld      a0,(PROC_REGS + REGS_PC)(tp)    /* Load pc */
        addi    a0,a0,4                         /* pc += 4 */
        sd      a0,(PROC_REGS + REGS_PC)(tp)    /* Save pc */

trap_resume_proc:
        /* Enable preemption */
        csrw    mstatus,MSTATUS_IE

        /* Restore all registers except tp, sp, gp */
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

_syscall_vector:
.option push
.option norvc
        /* Syscall using capabilities */
        j       syscall_read_cap
        j       syscall_move_cap
        j       syscall_delete_cap
        j       syscall_revoke_cap
        j       syscall_derive_cap
        j       syscall_invoke_cap
        /* Syscall not using capabilities */
        j       syscall_get_pid
        j       syscall_read_reg
        j       syscall_write_reg
        j       syscall_yield
.option pop
