/* See LICENSE file for copyright and license details. */

.section .text.entry

.align 4
.globl AsmTrapHandler
AsmTrapHandler:
        /* Swap stack pointers. */
        csrrw   sp,mscratch,sp
        /* Save user context. */
        sd      ra,(0*8)(sp)
        sd      t0,(1*8)(sp)
        sd      t1,(2*8)(sp)
        sd      t2,(3*8)(sp)
        sd      s0,(4*8)(sp)
        sd      s1,(5*8)(sp)

        /* Pointer to proc struct. */
        ld      s0,(-1*8)(sp)

        /* Trampoline base. */
        la      ra,AsmTrapoline
        /* Calculate trampoline entry offset (t0). */
        csrr    t0,mcause
        srli    t1,t0,59        /* If interrupt, t1 == 16. */
        slli    t0,t0,2
        add     t0,t0,t1
        /* Add offset to base. */
        add     ra,ra,t0
        /* Jump to trampoline entry. */
        jalr    ra,0(ra)

        /* Save pointer to proc struct. */
        sd      s0,(-1*8)(sp)

        /* Restore user context. */
        ld      s1,(5*8)(sp)
        ld      s0,(4*8)(sp)
        ld      t2,(3*8)(sp)
        ld      t1,(2*8)(sp)
        ld      t0,(1*8)(sp)
        ld      ra,(0*8)(sp)
        /* Swap stack pointers. */
        csrrw   sp,mscratch,sp
        mret

AsmSyscallHandler:
        /* Naive syscall handler (Assumes every ecall is a valid system call). */
        /* Increment pc. */
        csrr    t0,mepc
        addi    t0,t0,4
        csrw    mepc,t0
        /* Calculate trampoline entry offset. */
        andi    t0,a7,0xF       /* Mask syscall number (0-15). */
        slli    t0,t0,2
        /* Trampoline base. */
        la      t1,AsmSyspoline
        /* Add offset to base. */
        add     t0,t0,t1
        /* Jump to trampoline entry. */
        jalr    x0,0(t0)

.option push
.option norvc
.globl AsmTrapoline
AsmTrapoline:
/* Exceptions. */
        j       __hang                  #  0: Instruction address misaligned
        j       __hang                  #  1: Instruction access fault
        j       __hang                  #  2: Illegal instruction
        j       __hang                  #  3: Breakpoint
        j       __hang                  #  4: Load address misaligned
        j       __hang                  #  5: Load access fault
        j       __hang                  #  6: Store/AMO address misaligned
        j       __hang                  #  7: Store/AMO access fault
        j       AsmSyscallHandler       #  8: Environment call from U-mode
        j       __hang                  #  9: Environment call from S-mode
        j       __hang                  # 10: Reserved
        j       __hang                  # 11: Environment call from M-mode
        j       __hang                  # 12: Instruction page fault
        j       __hang                  # 13: Load page fault
        j       __hang                  # 14: Reserved
        j       __hang                  # 15: Store/AMO page fault
/* Interrupts. */
        j       __hang                  #  0: Reserved
        j       __hang                  #  1: Supervisor software interrupt
        j       __hang                  #  2: Reserved
        j       __hang                  #  3: Machine software interrupt
        j       __hang                  #  4: Reserved
        j       __hang                  #  5: Supervisor timer interrupt
        j       __hang                  #  6: Reserved
        j       __hang                  #  7: Machine timer interrupt
        j       __hang                  #  8: Reserved
        j       __hang                  #  9: Supervisor external interrupt
        j       __hang                  # 10: Reserved
        j       __hang                  # 11: Machine external interrupt
        j       __hang                  # 12: Reserved
        j       __hang                  # 13: Reserved
        j       __hang                  # 14: Reserved
        j       __hang                  # 15: Reserved

.globl AsmSyscallTrampoline
AsmSyspoline:
        j       __hang                  #  0:
        j       __hang                  #  1:
        j       __hang                  #  2:
        j       __hang                  #  3:
        j       __hang                  #  4:
        j       __hang                  #  5:
        j       __hang                  #  6:
        j       __hang                  #  7:
        j       __hang                  #  8:
        j       __hang                  #  9:
        j       __hang                  # 10:
        j       __hang                  # 11:
        j       __hang                  # 12:
        j       __hang                  # 13:
        j       __hang                  # 14:
        j       __hang                  # 15:
.option pop




__hang:
        ret
