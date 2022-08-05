// See LICENSE file for copyright and license details.
#include "preemption.h"
#include "sched.h"
#include "trap.h"

void exception_handler(uint64_t mcause, uint64_t mtval)
{
        preemption_disable();
        /* Save pc, sp, a0, a1 to trap frame */
        current->regs.ppc = current->regs.pc;
        current->regs.psp = current->regs.sp;
        current->regs.pa0 = current->regs.a0;
        current->regs.pa1 = current->regs.a1;
        /* Prepare for trap handler */
        current->regs.pc = current->regs.tpc;
        current->regs.sp = current->regs.tsp;
        current->regs.a0 = mcause;
        current->regs.a1 = mtval;
        trap_resume_proc();
}

#define MRET 0x0320000073

void illegal_instruction_handler(uint64_t mcause, uint64_t mtval)
{
        if (mtval == MRET) {
                /* Restore sp, pc, a0, a1 */
                preemption_disable();
                current->regs.sp = current->regs.psp;
                current->regs.pc = current->regs.ppc;
                current->regs.a0 = current->regs.pa0;
                current->regs.a1 = current->regs.pa1;
                trap_resume_proc();
        }
        exception_handler(mcause, mtval);
}
