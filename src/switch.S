.globl asm_switch_from_proc
.globl asm_switch_to_proc

.section .text.switch

asm_switch_from_proc:
        addi    sp,sp,-14*8
        sd      ra,(0*8)(sp)
        csrr    t0,mscratch
        sd      t0,(1*8)(sp)
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

        /* Swap process sp with core sp */
        amoswap.d sp,sp,(tp)

        /* void sched(void) */
        call    sched

asm_switch_to_proc:

        /* Swap process sp with core sp */
        amoswap.d sp,sp,(tp)

        /* Add memory protection here */

        ld      ra,(0*8)(sp)
        ld      t0,(1*8)(sp)
        csrw    mscratch,t0
        ld      s0,(2*8)(sp)
        ld      s1,(3*8)(sp)
        ld      s2,(4*8)(sp)
        ld      s3,(5*8)(sp)
        ld      s4,(6*8)(sp)
        ld      s5,(7*8)(sp)
        ld      s6,(8*8)(sp)
        ld      s7,(9*8)(sp)
        ld      s8,(10*8)(sp)
        ld      s9,(11*8)(sp)
        ld      s10,(12*8)(sp)
        ld      s11,(13*8)(sp)

        addi    sp,sp,14*8
        ret
