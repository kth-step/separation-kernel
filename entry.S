# See LICENSE file for copyright and license details.

.globl AsmTrapTrampoline
.extern AsmSyscallHandler

.section .text.entry

# Trap Trampoline {{{
.option push
.option norvc
.align 4
AsmTrapTrampoline:
        j       AsmExcptHandler         #  0: Reserved
        j       __dummy                 #  1: Supervisor software interrupt
        j       __dummy                 #  2: Reserved
        j       __dummy                 #  3: Machine software interrupt
        j       __dummy                 #  4: Reserved
        j       __dummy                 #  5: Supervisor timer interrupt
        j       __dummy                 #  6: Reserved
        j       __dummy                 #  7: Machine timer interrupt
        j       __dummy                 #  8: Reserved
        j       __dummy                 #  9: Supervisor external interrupt
        j       __dummy                 # 10: Reserved
        j       __dummy                 # 11: Machine external interrupt
        j       __dummy                 # 12: Reserved
        j       __dummy                 # 13: Reserved
        j       __dummy                 # 14: Reserved
        j       __dummy                 # 15: Reserved
.option pop
# }}}

# Exception Handler {{{
AsmExcptHandler:
        csrrw   sp,mscratch,sp          # Save user sp and load kernel sp.

        sd      ra,(0*8)(sp)            # Save part of user context.
        sd      t0,(1*8)(sp)
        sd      t1,(2*8)(sp)
        sd      t2,(3*8)(sp)
        sd      s0,(4*8)(sp)
        sd      s1,(5*8)(sp)

        ld      s0,(-1*8)(sp)           # Load pointer to proc struct.
        
        csrr    t0,mcause               # Fetch cause of exception.
        li      t1,8                    # 8 is system call cause.
        beq     t0,t1,__syscall         # If cause is syscall, goto __syscall, else __exception.

__exception:
        la      ra,AsmExcptTrampoline   # Load base address to exception trampoline.
        j       1f                      # Goto label 1 forward.

__syscall:
        la      ra,AsmSyscallTrampoline # Load base address to syscall trampoline.

        csrr    t0,mepc                 # Increment pc by 4.
        addi    t0,t0,4
        csrw    mepc,t0
        andi    t0,a7,0xF               # Mask syscall number so we get t0=[0-15].

1:      slli    t0,t0,2                 # Shift 2 left, t0 becomes offset to trampoline.
        add     ra,ra,t0                # Add offset to trampoline base address.
        jalr    ra,0(ra)                # Jump to the trampoline entry

        sd      s0,(-1*8)(sp)           # Save pointer to proc struct.

        ld      s1,(5*8)(sp)            # Restore parts of te user context.
        ld      s0,(4*8)(sp)
        ld      t2,(3*8)(sp)
        ld      t1,(2*8)(sp)
        ld      t0,(1*8)(sp)
        ld      ra,(0*8)(sp)
        csrrw   sp,mscratch,sp          # Save kernel sp and load user sp from scratch.
        mret                            # Return to user proc.
# }}}

# Exception Trampoline {{{
.option push
.option norvc
AsmExcptTrampoline:
        j       __dummy                 #  0: Instruction address misaligned
        j       __dummy                 #  1: Instruction access fault
        j       __dummy                 #  2: Illegal instruction
        j       __dummy                 #  3: Breakpoint
        j       __dummy                 #  4: Load address misaligned
        j       __dummy                 #  5: Load access fault
        j       __dummy                 #  6: Store/AMO address misaligned
        j       __dummy                 #  7: Store/AMO access fault
        j       __dummy                 #  8: Environment call from U-mode
        j       __dummy                 #  9: Environment call from S-mode
        j       __dummy                 # 10: Reserved
        j       __dummy                 # 11: Environment call from M-mode
        j       __dummy                 # 12: Instruction page fault
        j       __dummy                 # 13: Load page fault
        j       __dummy                 # 14: Reserved
        j       __dummy                 # 15: Store/AMO page fault
.option pop
# }}}

# Dummy {{{
__dummy:
        ret
# }}}
