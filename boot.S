# See LICENSE file for copyright and license details.

# Boot code {{{
.section .init
.extern stack
.globl _start
_start:
        la      t0,AsmTrapTrampoline
        ori     t0,t0,1
        csrw    mtvec,t0

        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop

        la      sp,_sp

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
