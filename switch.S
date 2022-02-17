# See LICENSE file for copyright and license details.

#include "offsets.h"

.globl AsmSwitchProc
.globl AsmStartProc

.section .text.switch


# AsmSwitchProc {{{
AsmSwitchProc:
        # Switches Process
        # Push context to process stack, gets new process stack and load new 
        # context from that process stack.
        # - tp: old process
        # - a0: new process

        # Push context to stack.
        addi    sp,sp,-(16*8)
        sd      ra,(0*8)(sp)
        csrr    t0,mscratch
        sd      t0,(1*8)(sp)      # Save kernel stack
        sd      s0,(2*8)(sp)
        sd      s1,(3*8)(sp) 
        sd      s2,(4*8)(sp) 
        sd      s3,(5*8)(sp) 
        sd      s4,(6*8)(sp) 
        sd      s5,(7*8)(sp) 
        sd      s6,(8*8)(sp) 
        sd      s7,(9*8)(sp) 
        sd      s8,(10*8)(sp) 
        sd      s9,(11*8)(sp) 
        sd      s10,(12*8)(sp) 
        sd      s11,(13*8)(sp) 
        sd      t5,(14*8)(sp) 
        sd      t6,(15*8)(sp) 

        # Save kernel stack to Process struct.
        csrr    t0,mepc
        sd      t0,(PROC_USER_PC)(tp)      # Save user pc
        sd      sp,(PROC_KERNEL_STACK)(tp) # Save kernel stack

AsmStartProc:
        mv      tp,a0
        # Get new kernel stack from Process struct
        ld      sp,(PROC_KERNEL_STACK)(tp) # Load kernel stack
        ld      t0,(PROC_USER_PC)(tp)      # Load user pc
        csrw    mepc,t0
        # Load pmp configuration
#        ld      t0,(PROC_PMPCFG0)(s0)
#        ld      a0,(PROC_PMPADDR0)(s0)
#        ld      a1,(PROC_PMPADDR1)(s0)
#        ld      a2,(PROC_PMPADDR2)(s0)
#        ld      a3,(PROC_PMPADDR3)(s0)
#        ld      a4,(PROC_PMPADDR4)(s0)
#        ld      a5,(PROC_PMPADDR5)(s0)
#        ld      a6,(PROC_PMPADDR6)(s0)
#        ld      a7,(PROC_PMPADDR7)(s0)
#        csrw    pmpcfg0,t0
#        csrw    pmpaddr0,a0
#        csrw    pmpaddr1,a1
#        csrw    pmpaddr2,a2
#        csrw    pmpaddr3,a3
#        csrw    pmpaddr4,a4
#        csrw    pmpaddr5,a5
#        csrw    pmpaddr6,a6
#        csrw    pmpaddr7,a7

        # Pop context from stack.
        ld      t6,(15*8)(sp) 
        ld      t5,(14*8)(sp) 
        ld      s11,(13*8)(sp) 
        ld      s10,(12*8)(sp) 
        ld      s9,(11*8)(sp) 
        ld      s8,(10*8)(sp) 
        ld      s7,(9*8)(sp) 
        ld      s6,(8*8)(sp) 
        ld      s5,(7*8)(sp) 
        ld      s4,(6*8)(sp) 
        ld      s3,(5*8)(sp) 
        ld      s2,(4*8)(sp) 
        ld      s1,(3*8)(sp) 
        ld      s0,(2*8)(sp)
        ld      t0,(1*8)(sp) 
        csrw    mscratch,t0
        ld      ra,(0*8)(sp)
        addi    sp,sp,(16*8)

        ret
# }}}
