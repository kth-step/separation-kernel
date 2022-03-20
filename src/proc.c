// See LICENSE file for copyright and license details.
#include "config.h"
#include "proc.h"
#include "stack.h"

extern void AsmTrapExit();
extern void user_code();

Process processes[N_PROC];

#define INIT_STACK_OFFSET 32
void InitProc(void) {
        for (int i = 0; i < N_PROC; i++) {
                Process *proc = &processes[i];
                proc->pid = i;
                proc->ksp = &proc_stack[i][STACK_SIZE/8-INIT_STACK_OFFSET];
                *proc->ksp = (uintptr_t)AsmTrapExit;
                proc->pc = (uintptr_t*)user_code;
        }
}
