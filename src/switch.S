.globl AsmSwitchToSched
.globl AsmSwitchToProc

.section .text.switch

AsmSwitchToSched:
        addi    sp,sp,-14*8
        /* Store return address. */
        /* Note: This return address belongs to
         * kernel process, user ra is stored in trap entry 
         */
        sd      ra,(0*8)(sp)
        /* Store user sp. */
        /* Note: When in debug mode, the kernel stores the
         * user sp in the trap entry, replacing mscratch with
         * zero to detect exceptions in the kernel.
         */
        csrr    t0,mscratch
        sd      t0,(1*8)(sp)
        /* Store saved registers. */
        /* Note: The saved registers has user OR kernel data,
         * if the saved registers has kernel data, then the
         * user data has been stored in the stack by a C function
         * as per calling conventions. */
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

        /* void Sched(void) */
        call    Sched

AsmSwitchToProc:

        /* Swap process sp with core sp */
        amoswap.d sp,sp,(tp)

        /* TODO: Add memory protection here */

        /* Load return address. */
        ld      ra,(0*8)(sp)
        /* Load user sp. */
        ld      t0,(1*8)(sp)
        csrw    mscratch,t0
        /* Load saved registers. */
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
