// See LICENSE file for copyright and license details.
#include "preemption.h"
#include "sched.h"
#include "trap.h"

#define MRET 0x0320000073ull
#define ILLEGAL_INSTRUCTION 2ull

void exception_handler(void)
{
        uint64_t mcause = read_csr(mcause);
        uint64_t mtval = read_csr(mtval);

        preemption_disable();
        if (mcause == ILLEGAL_INSTRUCTION && mtval == MRET) {
                /* Restore sp, pc, a0, a1 */
                current->regs.sp = current->regs.psp;
                current->regs.pc = current->regs.ppc;
                current->regs.a0 = current->regs.pa0;
                current->regs.a1 = current->regs.pa1;
        } else {
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
        }
        trap_resume_proc();
}
