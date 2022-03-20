# See LICENSE file for copyright and license details.

.globl AsmTrapHandler
.globl AsmRestoreUser

.extern ecall_get_pid

.section .text.entry

# System Call Trampoline {{{
__syscall_trampoline:
        .option push
        .option norvc
        j       ecall_get_pid           #  0:
        j       __dummy                 #  1:
        j       __dummy                 #  2:
        j       __dummy                 #  3:
        j       __dummy                 #  4:
        j       __dummy                 #  5:
        j       __dummy                 #  6:
        j       __dummy                 #  7:
        j       __dummy                 #  8:
        j       __dummy                 #  9:
        j       __dummy                 # 10:
        j       __dummy                 # 11:
        j       __dummy                 # 12:
        j       __dummy                 # 13:
        j       __dummy                 # 14:
        j       __dummy                 # 15:
        .option pop
# }}}


# Exception Handler {{{
.balign 16
AsmTrapHandler:
        csrrw   sp,mscratch,sp          # Save user sp and load kernel sp.
        sd      a0,(0*8)(sp)
        sd      a1,(1*8)(sp)
        sd      a2,(2*8)(sp)
        sd      a3,(3*8)(sp)
        sd      a4,(4*8)(sp)
        sd      a5,(5*8)(sp)
        sd      a6,(6*8)(sp)
        sd      a7,(7*8)(sp)
        sd      t0,(8*8)(sp)
        sd      t1,(9*8)(sp)
        sd      t2,(10*8)(sp)
        sd      t3,(11*8)(sp)
        sd      t4,(12*8)(sp)
        sd      ra,(13*8)(sp)            # Save part of user context.
        sd      tp,(14*8)(sp)
        sd      gp,(15*8)(sp)
        ld      tp,(-1*8)(sp)           # Load pointer to proc struct.
        ld      gp,(-2*8)(sp)           # Load global pointer.
        csrr    t0,mip
        andi    t0,t0,(1<<7)
        beqz    t0,1f
        call    AsmReleaseProc    
1:      csrr    t0,mcause               # Fetch cause of exception.
        li      t1,8
        beq     t0,t1,__syscall         # If cause is syscall, goto __syscall, else __exception.
        li      t1,2                    # 2 is illegal instruction.
        csrr    t2,mtval                # Load trap value
        bne     t0,t1,__exception       
__illegal_instruction:
        li      t1,0                    # Load op code of uret
        bne     t1,t2,__exception       # If not uret, do user exception.
__handle_uret:
        # TODO: Handler uret
        j       __restore_user
__exception:
        # TODO: Handle user exceptions
        j       __restore_user          # Goto label 1 forward.
__syscall:
        la      ra,__syscall_trampoline # Load base address to syscall trampoline.
        mv      a0,sp

        csrr    t0,mepc                 # Increment pc by 4.
        addi    t0,t0,4
        csrw    mepc,t0
        andi    t0,a7,0xF               # Mask syscall number so we get t0=[0-15].
        slli    t0,t0,2                 # Shift 2 left, t0 becomes offset to trampoline.
        add     ra,ra,t0                # Add offset to trampoline base address.
        jalr    ra,0(ra)                # Jump to the trampoline entry

AsmRestoreUser:      
__restore_user:
        csrr    t0,mip
        andi    t0,t0,(1<<7)
        beqz    t0,1f
        call    AsmReleaseProc
1:      sd      gp,(-2*8)(sp)           # Save global pointer.
        sd      tp,(-1*8)(sp)           # Save pointer to proc struct.
        ld      gp,(15*8)(sp)           # Load part of user context.
        ld      tp,(14*8)(sp)
        ld      ra,(13*8)(sp)
        ld      t4,(12*8)(sp)
        ld      t3,(11*8)(sp)
        ld      t2,(10*8)(sp)
        ld      t1,(9*8)(sp)
        ld      t0,(8*8)(sp)
        ld      a7,(7*8)(sp)
        ld      a6,(6*8)(sp)
        ld      a5,(5*8)(sp)
        ld      a4,(4*8)(sp)
        ld      a3,(3*8)(sp)
        ld      a2,(2*8)(sp)
        ld      a1,(1*8)(sp)
        ld      a0,(0*8)(sp)

        csrrw   sp,mscratch,sp          # Save kernel sp and load user sp from scratch.
        mret                            # Return to user proc.
# }}}

# Dummy {{{
__dummy:
        ret
# }}}
