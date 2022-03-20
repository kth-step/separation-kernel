// See LICENSE file for copyright and license details.
#include "config.h"
#include "proc.h"
#include "lock.h"
#include "sched.h"
#include "stack.h"

#define INIT_STACK_OFFSET 33
extern void AsmStartProc(Process*);
extern void AsmRestoreUser();
extern void user_code();
volatile int init_lock = 0;
void init(void) {
        for (int i = 0; i < N_PROC; i++) {
                Process *proc = &processes[i];
                proc->pid = i;
                proc->kernelSP = &proc_stack[i+1][STACK_SIZE/8]-INIT_STACK_OFFSET;
                proc->userPC = (void*)user_code;
                *proc->kernelSP = (uintptr_t)AsmRestoreUser;
        }
        __asm__("fence rw,rw");
        init_lock = N_PROC;
}

