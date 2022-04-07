// See LICENSE file for copyright and license details.

#include <assert.h>
#include <stdint.h>

#include "capabilities.h"
#include "proc.h"
#include "stack.h"

/* Get process ID */
void SyscallPid(void) {
        current->args[0] = current->pid;
}

/* Initialize a process using a supervisor. */
void SyscallInit(uintptr_t cid_sup, uintptr_t cid_pmp) {
        current->args[0] = 0;
        /* Check if valid capability index */
        Cap *cap_sup = &current->cap_table[cid_sup % N_CAPS];
        Cap *cap_pmp = &current->cap_table[cid_pmp % N_CAPS];

        /* Check if capability type is supervisor */
        if (!cap_is_type(cap_sup, CAP_SUPERVISOR) ||
            !cap_is_type(cap_pmp, CAP_PMP_ENTRY))
                return;
        /* Get the superviser cap */
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        assert(sup.pid < N_PROC);
        /* Get the supervisee */
        Proc *proc = &processes[sup.pid];
        /* Check if the supervisee has halted. */
        if (proc->state != PROC_HALTED)
                return;

        /* Now everything is set */
        /* Clean stack. */
        for (int i = 0; i < (STACK_SIZE / 8); i++)
                proc_stack[sup.pid][i] = 0;
        /* Revoke then delete all capabilities */
        for (int i = 0; i < N_CAPS; i++) {
                CapRevoke(&proc->cap_table[i]);
                CapDelete(&proc->cap_table[i]);
        }

        /* Sets PMP capability */
        /* TODO: Unload PMP cap in supervisor */
        /* Move PMP capabilitiy to supervisee */
        CapMove(&proc->cap_table[0], cap_pmp);
        /* TODO: Load PMP cap in supervisor */
        /* Set success */
        current->args[0] = 1;
}

/* Read a capability. */
void SyscallReadCap(uintptr_t cid) {
        Cap *cap = &current->cap_table[cid % N_CAPS];
        /* Read fields first */
        current->args[0] = cap->field0;
        current->args[1] = cap->field1;
        /* Fence so we read first then check if deleted */
        fence(r, r);
        if (cap_is_deleted(cap)) {
                /* If deleted, then set args. */
                current->args[0] = 0;
                current->args[1] = 0;
        }
}

/* Move a capability internally */
void SyscallMove(uintptr_t cid_dest, uintptr_t cid_src) {
        if (cid_dest == cid_src) {
                current->args[0] = 0;
                return;
        }
        Cap *cap_dest = &current->cap_table[cid_dest % N_CAPS];
        Cap *cap_src = &current->cap_table[cid_src % N_CAPS];
        if (!cap_is_deleted(cap_dest) || cap_is_deleted(cap_src)) {
                current->args[0] = 0;
                return;
        }
        current->args[0] = CapMove(cap_dest, cap_src);
}

/* Revoke on a capability, deleting children */
void SyscallRevoke(uintptr_t cid) {
        Cap *cap = &current->cap_table[cid % N_CAPS];
        if (cap_is_deleted(cap))
                current->args[0] = 0;
        CapRevoke(cap);
        current->args[0] = 1;
}

/* Deletes a capability */
void SyscallDelete(uintptr_t cid) {
        Cap *cap = &current->cap_table[cid % N_CAPS];
        current->args[0] = CapDelete(cap);
}

/* Create a slice of a capability */
void SyscallSlice(uintptr_t cid_src, uintptr_t cid_dest, uint64_t field0,
                  uint64_t field1) {
        current->args[0] = 0;
        Cap *cap_src = &current->cap_table[cid_src % N_CAPS];
        Cap *cap_dest = &current->cap_table[cid_dest % N_CAPS];
        if (!cap_is_deleted(cap_dest) || cap_is_deleted(cap_src))
                return;
        cap_dest->field0 = field0;
        cap_dest->field1 = field1;
        if (!cap_is_valid(cap_dest) || !cap_is_child(cap_src, cap_dest))
                return;
        /* If parent has a child, error. */
        Cap *cap_next = cap_src->next;
        if (cap_next != NULL && cap_is_child(cap_src, cap_next))
                return;
        /* Validate slices */
        current->args[0] = CapInsert(cap_src, cap_dest);
}

/* Creates two slices of a capability */
void SyscallSplit(uintptr_t cid_src, uintptr_t cid_dest0, uint64_t field00,
                  uint64_t field01, uintptr_t cid_dest1, uint64_t field10,
                  uint64_t field11) {
        current->args[0] = 0;
        Cap *cap_src = &current->cap_table[cid_src % N_CAPS];
        Cap *cap_dest0 = &current->cap_table[cid_dest0 % N_CAPS];
        Cap *cap_dest1 = &current->cap_table[cid_dest1 % N_CAPS];
        if (!cap_is_deleted(cap_dest0) || !cap_is_deleted(cap_dest1))
                return;
        cap_dest0->field0 = field00;
        cap_dest0->field1 = field01;
        cap_dest1->field0 = field10;
        cap_dest1->field1 = field11;
        /* Validate slices */
        if (!cap_is_valid(cap_dest0) || !cap_is_valid(cap_dest1))
                return;
        if (!cap_is_child(cap_src, cap_dest0) ||
            !cap_is_child(cap_src, cap_dest1))
                return;
        Cap *next = cap_src->next;
        if (next != NULL && cap_is_child(cap_src, next))
                return;
        if (cap_is_type(cap_dest0, CAP_MEMORY_SLICE)) {
                if (cap_is_type(cap_dest1, CAP_MEMORY_SLICE) &&
                    cap_is_intersect_ms_ms(cap_get_memory_slice(cap_dest0),
                                           cap_get_memory_slice(cap_dest1))) {
                        return;
                } else if (cap_is_intersect_ms_pe(
                               cap_get_memory_slice(cap_dest0),
                               cap_get_pmp_entry(cap_dest1))) {
                        return;
                }
        } else if (cap_is_type(cap_dest0, CAP_PMP_ENTRY)) {
                if (cap_is_type(cap_dest1, CAP_MEMORY_SLICE) &&
                    cap_is_intersect_ms_pe(cap_get_memory_slice(cap_dest1),
                                           cap_get_pmp_entry(cap_dest0))) {
                        return;
                } else if (cap_is_intersect_pe_pe(
                               cap_get_pmp_entry(cap_dest0),
                               cap_get_pmp_entry(cap_dest1))) {
                        return;
                }
        } else if (cap_is_type(cap_dest0, CAP_TIME_SLICE) &&
                   cap_is_type(cap_dest1, CAP_TIME_SLICE) &&
                   cap_is_intersect_ts_ts(cap_get_time_slice(cap_dest0),
                                          cap_get_time_slice(cap_dest1))) {
                return;
        }
        current->args[0] =
            CapInsert(cap_src, cap_dest0) && CapInsert(cap_src, cap_dest1);
}

/* Supervisor moves a capability between itself and a supervisee */
void SyscallSupMove(uint64_t cid_sup, uint64_t cid_dest, uint64_t cid_src,
                    uint64_t grant) {
        current->args[0] = 0;
        Cap *cap_sup = &current->cap_table[cid_sup % N_CAPS];
        if (!cap_is_type(cap_sup, CAP_SUPERVISOR))
                return;
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        Cap *cap_dest, *cap_src;
        if (grant) {
                cap_dest = &processes[sup.pid].cap_table[cid_dest % N_CAPS];
                cap_src = &current->cap_table[cid_src % N_CAPS];
        } else {
                cap_dest = &current->cap_table[cid_dest % N_CAPS];
                cap_src = &processes[sup.pid].cap_table[cid_src % N_CAPS];
        }

        if (!cap_is_deleted(cap_dest) || cap_is_deleted(cap_src))
                return;
        current->args[0] = CapMove(cap_dest, cap_src);
}

/* Supervisor halts a process */
void SyscallSupHalt(uint64_t cid_sup);

/* Supervisor resumes a process */
void SyscallSupResume(uint64_t cid_sup);
