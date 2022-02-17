// See LICENSE file for copyright and license details.

#include "types.h"
#include "proc.h"

#define OFFSET(NAME, STRUCT, MEMBER) \
        __asm__("#define "#NAME"\t%0\n"::"i"(__builtin_offsetof(STRUCT, MEMBER)))

void foo(void) {
        OFFSET(PROC_KERNEL_STACK,   Process, kernelSP); 
        OFFSET(PROC_USER_PC,        Process, userPC); 
        OFFSET(PROC_TRAP_REGISTERS, Process, trapRegisters);
        OFFSET(PROC_CAPABILITIES,   Process, capabilities);
        OFFSET(PROC_PMPCFG0,  Process, pmpConfig.pmpcfg0);
        OFFSET(PROC_PMPADDR0, Process, pmpConfig.pmpaddr0);
        OFFSET(PROC_PMPADDR1, Process, pmpConfig.pmpaddr1);
        OFFSET(PROC_PMPADDR2, Process, pmpConfig.pmpaddr2);
        OFFSET(PROC_PMPADDR3, Process, pmpConfig.pmpaddr3);
        OFFSET(PROC_PMPADDR4, Process, pmpConfig.pmpaddr4);
        OFFSET(PROC_PMPADDR5, Process, pmpConfig.pmpaddr5);
        OFFSET(PROC_PMPADDR6, Process, pmpConfig.pmpaddr6);
        OFFSET(PROC_PMPADDR7, Process, pmpConfig.pmpaddr7);

        OFFSET(TR_TRAP_CAUSE, TrapRegisters, trapCause);
        OFFSET(TR_TRAP_VALUE, TrapRegisters, trapValue);
}
