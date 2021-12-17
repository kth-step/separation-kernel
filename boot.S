# See LICENSE file for copyright and license details.

# Boot code {{{
.section .init
.globl _start
_start:
        la      t0,AsmTrapTrampoline
        ori     t0,t0,1
        csrw    mtvec,t0

        la      sp,_sp
        addi    sp,sp,256

        la      t0,user_code
        csrw    mepc,t0
        csrw    mscratch,sp
        csrw    mstatus,x0
        li      t0,0xF
        li      t1,-1
        csrw    pmpcfg0,t0
        csrw    pmpaddr0,t1
        mret
# }}}
