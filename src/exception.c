#include "trap.h"
#include "sched.h"

void ExceptionHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval) {
        SchedDisablePreemption();
        /* Save pc, sp, a0, a1 to trap frame */
        tf->ppc = tf->pc;
        tf->psp = tf->sp;
        tf->pa0 = tf->a0;
        tf->pa1 = tf->a1;
        /* Prepare for trap handler */
        tf->pc = tf->tpc;
        tf->sp = tf->tsp;
        tf->a0 = mcause;
        tf->a1 = mtval;
        /* If tf->tpc & 1, then vectored mode */
        if (tf->tpc & 1)
                tf->pc += mcause * 4;
        SchedEnablePreemption();
}

#define URET 0x0020000073
#define SRET 0x0120000073
#define MRET 0x0320000073

void IllegalInstructionHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval) {
        if (mtval == URET || mtval == SRET || mtval == MRET) {
                SchedDisablePreemption();
                /* Restore sp, pc, a0, a1 */
                tf->sp = tf->psp;
                tf->pc = tf->ppc;
                tf->a0 = tf->pa0;
                tf->a1 = tf->pa1;
                tf->pc += 4;
                SchedEnablePreemption();
        } else {
                ExceptionHandler(tf, mcause, mtval);
        }
}
