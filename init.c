#include "config.h"
#include "proc.h"

#define INIT_STACK_OFFSET 36
extern void AsmStartProc(Process*);
extern void AsmRestoreUser();
extern void user_code();
void init(void) {
        for (int i = 0; i < N_PROC; i++) {
                Process *proc = &processes[i];
                proc->pid = i+1;
                uint8_t *stack_top = &stack[i][STACK_SIZE];
                proc->kernelSP = stack_top - INIT_STACK_OFFSET;
                proc->userPC = (uint8_t*)user_code;
                *((uintptr_t*)proc->kernelSP) = (uintptr_t)AsmRestoreUser;
        }
        AsmStartProc(processes);
}

