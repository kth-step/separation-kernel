#include "trap.h"
#include "sched.h"

void ExceptionHandler(struct registers *regs, uint64_t mcause, uint64_t mtval) {
        SchedDisablePreemption();
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
        SchedEnablePreemption();
}

#define URET 0x0020000073
#define SRET 0x0120000073
#define MRET 0x0320000073

void IllegalInstructionHandler(struct registers *regs, uint64_t mcause, uint64_t mtval) {
        if (mtval == URET || mtval == SRET || mtval == MRET) {
                SchedDisablePreemption();
                /* Restore sp, pc, a0, a1 */
                regs->sp = regs->psp;
                regs->pc = regs->ppc;
                regs->a0 = regs->pa0;
                regs->a1 = regs->pa1;
                SchedEnablePreemption();
        } else {
                ExceptionHandler(regs, mcause, mtval);
        }
}
