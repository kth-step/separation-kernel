# See LICENSE file for copyright and license details.
#include "s3k_consts.h"

#define MCAUSE_U_ECALL 8
#define MSTATUS_IE 8

.globl trap_vector
.globl trap_resume_proc
.globl trap_yield
.globl trap_syscall_exit
.globl trap_syscall_exit2
.globl trap_syscall_exit3

#include "config.h"
#include "macros.S"

.section .text.trap_entry
.align 8
trap_vector:
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

        csrr    t3,mcause
        
        /* Must be timer interrupt, yield */
        bltz    t3,sched_yield

        /* Enable preemption */
        csrw    mstatus,MSTATUS_IE

        /* Check if ecall from U mode */ 
        li      t0,MCAUSE_U_ECALL
        bne     t3,t0,exception
        /* Check that sysnr is correct */
        li      t0,NUM_OF_SYSNR
        bge     a7,t0,exception

syscall:

        /* if syscall number < S3K_SYSNR_READ_CAP,
         * we use no capabilities */
        li      t0,S3K_SYSNR_READ_CAP
        blt     a7,t0,syscall_jump

syscall_get_capability:
        /* a0 := a0 mod N_CAPS */
        /* Relies on N_CAP = 2^i, for some i */
        andi    a0,a0,(N_CAPS - 1)

        /* Get pointer to cap_node */
        /* t0 = current->cap_table */
        ld      t0,(43*8)(tp)
        /* Each cap_node is 32 byte wide (2^5 byte wide) */
        slli    t1,a0,5
        /* a2 = current->cap_table[a0] */
        add     a2,t0,t1 

        /* Get cap = {a0, a1} */
        ld      a0,16(a2)
        ld      a1,24(a2)

        /* Get cap first, then check if it has been deleted */
        fence   r,r

        /* If cap_node->prev == NULL, return error S3K_EMPTY */
        ld      t4,0(a2)
        beqz    t4,syscall_empty

syscall_jump:
        /* Jump to syscall handler */
        la      ra,syscall_vector
        slli    t0,a7,2
        add     ra,ra,t0
        jalr    zero,(ra)

exception:
        /* Enable preemption */
        csrr    a1,mtval
        la      ra,exception_vector
        mv      a0,t3
        slli    t0,a0,2
        add     ra,ra,t0
        jalr    zero,(ra)

syscall_empty:
        li      a0,S3K_EMPTY

trap_syscall_exit:
        /* Disable preemption */
        csrw    mstatus,0
trap_syscall_exit2:
        /* Save error code to a0 */
        sd      a0,(10*8)(tp)
trap_syscall_exit3:
        /* Increment pc */
        ld      t0,(0*8)(tp)
        addi    t0,t0,4
        sd      t0,(0*8)(tp)

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


.option push
.option norvc   /* Prevent compressed instructions */
exception_vector:
        j       exception_handler  /* 0: Instruction address misaligned */
        j       exception_handler  /* 1: Instruction access fault */
        j       illegal_instruction_handler  /* 2: Illegal instruction */
        j       exception_handler  /* 3: Breakpoint */
        j       exception_handler  /* 4: Load address misaligned */
        j       exception_handler  /* 5: Load access fault */
        j       exception_handler  /* 6: Store/AMO address misaligned */
        j       exception_handler  /* 7: Store/AMO access fault */
        j       exception_handler  /* 8: Environment call from U-mode */
        j       hang  /* 9: Environment call from S-mode */
        j       hang  /* 10: Reserved */
        j       hang  /* 11: Environment call from M-mode */
        j       hang  /* 12: Instruction page fault */
        j       hang  /* 13: Load page fault */
        j       hang  /* 14: Reserved */
        j       hang  /* 15: Store/AMO page fault */
syscall_vector:
        j       syscall_get_pid
        j       syscall_read_reg
        j       syscall_write_reg
        j       syscall_yield
        j       syscall_read_cap
        j       syscall_move_cap
        j       syscall_delete_cap
        j       syscall_revoke_cap
        j       syscall_derive_cap
        j       syscall_invoke_cap
.option pop

hang:
        j       hang

mhang:
        j       mhang
