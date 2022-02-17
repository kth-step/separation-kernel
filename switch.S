# See LICENSE file for copyright and license details.

#include "offsets.h"

.globl AsmSwitchProc

.section .text.switch

# AsmSwitchProc {{{
AsmSwitchProc:
        # Switches Process
        # Push context to process stack, gets new process stack and load new 
        # context from that process stack.
        # - s0: old process
        # - t0: new process

        # Push context to stack.
        addi    sp,sp,-(16*8)
        csrr    t1,mscratch
        sd      t1,(0*8)(sp)
        csrr    t2,mepc
        sd      t2,(1*8)(sp)
        sd      ra,(2*8)(sp)
        sd      gp,(3*8)(sp)
        sd      tp,(4*8)(sp) 
        sd      s2,(5*8)(sp) 
        sd      s3,(6*8)(sp) 
        sd      s4,(7*8)(sp) 
        sd      s5,(8*8)(sp) 
        sd      s6,(9*8)(sp) 
        sd      s7,(10*8)(sp) 
        sd      s8,(11*8)(sp) 
        sd      s9,(12*8)(sp) 
        sd      s10,(13*8)(sp) 
        sd      s11,(14*8)(sp) 

        # Save kernel stack to Process struct.
        sd      sp,(PROC_KERNEL_STACK)(s0)      # Save kernel stack

        # Set pointer to new process to s0.
        mv      s0,t0

        # Get new kernel stack from Process struct
        ld      sp,(PROC_KERNEL_STACK)(s0)

        # Load pmp configuration
        ld      t0,(PROC_PMPCFG0)(s0)
        ld      a0,(PROC_PMPADDR0)(s0)
        ld      a1,(PROC_PMPADDR1)(s0)
        ld      a2,(PROC_PMPADDR2)(s0)
        ld      a3,(PROC_PMPADDR3)(s0)
        ld      a4,(PROC_PMPADDR4)(s0)
        ld      a5,(PROC_PMPADDR5)(s0)
        ld      a6,(PROC_PMPADDR6)(s0)
        ld      a7,(PROC_PMPADDR7)(s0)
        csrw    pmpcfg0,t0
        csrw    pmpaddr0,a0
        csrw    pmpaddr1,a1
        csrw    pmpaddr2,a2
        csrw    pmpaddr3,a3
        csrw    pmpaddr4,a4
        csrw    pmpaddr5,a5
        csrw    pmpaddr6,a6
        csrw    pmpaddr7,a7

        # Pop context from stack.
        ld      s11,(14*8)(sp) 
        ld      s10,(13*8)(sp) 
        ld      s9,(12*8)(sp) 
        ld      s8,(11*8)(sp) 
        ld      s7,(10*8)(sp) 
        ld      s6,(9*8)(sp) 
        ld      s5,(8*8)(sp) 
        ld      s4,(7*8)(sp) 
        ld      s3,(6*8)(sp) 
        ld      s2,(5*8)(sp) 
        ld      tp,(4*8)(sp) 
        ld      gp,(3*8)(sp)
        ld      ra,(2*8)(sp)
        ld      t2,(1*8)(sp)
        csrw    mepc,t2
        ld      t1,(0*8)(sp)
        csrw    mscratch,t1
        addi    sp,sp,(16*8)

        ret
# }}}
