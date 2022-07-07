#include "trap.h"

void ExceptionHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval) {
        tf->cause = mcause;
        uint64_t sp = tf->sp;
        tf->sp = tf->esp;
        tf->esp = sp;

        uint64_t pc = tf->pc;
        tf->pc = tf->epc;
        tf->epc = pc;
}

#define URET 0x0020000073
#define SRET 0x0120000073
#define MRET 0x0320000073

void IllegalInstructionHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval) {
        if (mtval == URET || mtval == SRET || mtval == MRET) {
                uint64_t sp = tf->sp;
                tf->sp = tf->esp;
                tf->esp = sp;

                uint64_t pc = tf->pc;
                tf->pc = tf->epc;
                tf->epc = pc;
        } else {
                ExceptionHandler(tf, mcause, mtval);
        }
}
