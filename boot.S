# See LICENSE file for copyright and license details.

# Boot code {{{
.section .init
.extern stack
.globl _start
_start:

        # Initialize global pointer and stack pointer.
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop
        la      sp,_sp
        
        # Remove memory protection
        li      t0,0xF
        li      t1,-1
        csrw    pmpcfg0,t0
        csrw    pmpaddr0,t1

        # Setup trap handler 
        la      t0,AsmTrapTrampoline
        ori     t0,t0,1
        csrw    mtvec,t0
        csrw    mstatus,x0

        call    init
# }}}
