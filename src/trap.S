# See LICENSE file for copyright and license details.
#include "s3k_consts.h"

#define MCAUSE_U_ECALL 8
#define MSTATUS_IE 8

.globl trap_entry
.globl trap_syscall_exit1
.globl trap_syscall_exit2
.globl trap_resume_proc

#include "config.h"
#include "macros.S"

.section .text.trap_entry
.align 8
trap_entry:
        csrrw   tp,mscratch,tp
        bnez    tp,1f
        csrrw   tp,mscratch,zero
        /* Must be timer interrupt (or kernel bug), yield */
        j       sched_yield
1:
        /* Save all registers except tp */
        save_context

        /* Save tp */
        csrrw   t0,mscratch,zero
        sd      t0,(4*8)(tp)

        /* Load kernel gp */
        load_gp
        /* Load kernel sp */
        load_sp

        csrr    t0,mcause
        
        /* Must be timer interrupt, yield */
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
        bge     a7,t0,2f
        li      t0,S3K_SYSNR_INVOKE_CAP
        blt     t0,a7,1f

        andi    a0,a0,(N_CAPS - 1)
        slli    a0,a0,5
        ld      t0,(43*8)(tp)
        add     a0,a0,t0
        ld      a1,16(a0)
        ld      a2,24(a0)
        fence
        ld      t0,0(a0)
        beqz    t0,2f

        /* Jump to syscall handler */
1:      la      s0,syscall_vector
        slli    t0,a7,2
        add     s0,s0,t0
2:      jalr    s0

/* Return syscall with error code in a0, and turning off interrupts */
trap_syscall_exit1:
        csrw    mstatus,0

/* Return syscall with error code in a0, and not turning off interrupts */
trap_syscall_exit2:
        sd      a0,(10*8)(tp)
        ld      a0,(0*8)(tp)
        addi    a0,a0,4
        sd      a0,(0*8)(tp)

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
        ld      sp,(2*8)(tp)
        ld      gp,(3*8)(tp)
        ld      tp,(4*8)(tp)
        mret

syscall_vector:
.option push
.option norvc
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
