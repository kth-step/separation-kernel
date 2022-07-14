#include "preemption.h"
#include "sched.h"
#include "trap.h"

void exception_handler(struct registers* regs, uint64_t mcause, uint64_t mtval)
{
        preemption_disable();
        /* Save pc, sp, a0, a1 to trap frame */
        regs->ppc = regs->pc;
        regs->psp = regs->sp;
        regs->pa0 = regs->a0;
        regs->pa1 = regs->a1;
        /* Prepare for trap handler */
        regs->pc = regs->tpc;
        regs->sp = regs->tsp;
        regs->a0 = mcause;
        regs->a1 = mtval;
        preemption_enable();
}

#define URET 0x0020000073
#define SRET 0x0120000073
#define MRET 0x0320000073

void illegal_instruction_handler(struct registers* regs, uint64_t mcause, uint64_t mtval)
{
        if (mtval == URET || mtval == SRET || mtval == MRET) {
                /* Restore sp, pc, a0, a1 */
                preemption_disable();
                regs->sp = regs->psp;
                regs->pc = regs->ppc;
                regs->a0 = regs->pa0;
                regs->a1 = regs->pa1;
                preemption_enable();
        } else {
                exception_handler(regs, mcause, mtval);
        }
}
