// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "cap.h"
#include "cap_util.h"
#include "config.h"

/** Proc
 * This is the Process Control Block (PCB). We store pointers
 * register and process's state in this struct.
 */
typedef struct proc Proc;
/** ProcState
 * Describes the state of a process.
 */
typedef enum proc_state ProcState;

enum proc_state { PROC_HALTED, PROC_SUSPENDED, PROC_RUNNING, PROC_BLOCKED };

struct proc {
        /** Kernel stack pointer.
         *
         * When a process is running, we store the core's stack pointer
         * (pointer to core_stack) to ksp.
         *
         * When the process is not running, i.e., when the scheduler or some
         * other process is running, then we store the process's stack pointer
         * (pointer to proc_stack) to ksp.
         */
        uintptr_t *ksp;
        /** Process identifier.
         * This is the process's ID used for identification during
         * inter-process communication.
         */
        uintptr_t pid;
        /** Capability table.
         * Pointer to the capability table.
         */
        Cap *cap_table;
        /** Argument registers.
         * We store the argument registers a0-a7 in the args array,
         * this simplifies inter-process communication.
         */
        uintptr_t args[8];
        uintptr_t pc;

        /* The pmp configurations are stored in these capabilities */
        /* pmp_table[i].data[1] = pmpicfg | pmpaddri */
        Cap pmp_table[8];

        /** Process state.
         * TODO: Comment
         */
        ProcState state;
        bool halt;

        /* If listen_channel != -1, the process is waiting for a message from
         * the channel */
        int listen_channel;
};

/** Processes
 * We have a static number of processes N_PROC. The initial process is always
 * process 0.
 */
extern Proc processes[N_PROC];

/** Current process
 * This is a core local variable. If the core has a process, then current
 * points to the claimed process, otherwise it is NULL.
 */
register Proc *current __asm__("tp");

/** Initialize process.
 * This initializes all processes, setting their PID, stack pointer, and
 * entry points.
 */
void ProcInitProcesses(void);

void ProcHalt(Proc *proc);
void ProcReset(int pid);

static inline bool ProcLoadPmp(Proc *proc, CapPmpEntry pe, Cap *cap,
                               uint64_t index) {
        if (index > 8)
                return -1;
        Cap *child = &proc->pmp_table[index];
        CapLoadedPmp lp_child = cap_mk_loaded_pmp(pe.addr, pe.rwx);
        if (!cap_set(child, cap_serialize_loaded_pmp(lp_child)))
                return 0;
        if (!CapAppend(child, cap)) {
                child->data[1] = 0;
                return 0;
        }
        return 1;
}

static inline bool ProcUnloadPmp(Proc *proc, uint64_t index) {
        if (index > 8)
                return -1;
        return CapDelete(&proc->pmp_table[index]);
}
