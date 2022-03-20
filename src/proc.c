// See LICENSE file for copyright and license details.
#include "config.h"
#include "proc.h"
#include "stack.h"

extern void asm_trap_entry();
extern void asm_restore_user();
extern void user_code();

Process processes[N_PROC];

#define INIT_STACK_OFFSET 32
void init_proc(void) {
        for (int i = 0; i < N_PROC; i++) {
                Process *proc = &processes[i];
                proc->pid = i;
                proc->ksp = &proc_stack[i][STACK_SIZE/8-INIT_STACK_OFFSET];
                *proc->ksp = (uintptr_t)asm_restore_user;
                proc->pc = (uintptr_t*)user_code;
        }
}
