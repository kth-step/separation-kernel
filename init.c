// See LICENSE file for copyright and license details.
#include "inc/config.h"
#include "inc/proc.h"
#include "inc/lock.h"
#include "inc/sched.h"
#include "inc/stack.h"

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

