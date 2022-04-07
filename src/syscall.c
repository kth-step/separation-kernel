// See LICENSE file for copyright and license details.

#include <assert.h>
#include <stdint.h>

#include "capabilities.h"
#include "proc.h"
#include "stack.h"

void SyscallPid(void) {
        current->args[0] = current->pid;
}

void SyscallInit(uintptr_t cid_sup, uintptr_t cid_pmp) {
        /* Check if valid capability index */
        Cap *cap_sup = &current->cap_table[cid_sup % N_CAPS];
        Cap *cap_pmp = &current->cap_table[cid_pmp % N_CAPS];

        /* Check if capability type is supervisor */
        if (cap_is_type(cap_sup, CAP_SUPERVISOR) ||
            cap_is_type(cap_pmp, CAP_PMP_ENTRY)) {
                current->args[0] = 1;
                return;
        }
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        assert(sup.pid < N_PROC);
        Proc *proc = &processes[sup.pid];
        /* Check if process has halted. */
        if (proc->state == PROC_HALTED) {
                current->args[0] = 1;
        }
        /* Clean stack. */
        for (int i = 0; i < (STACK_SIZE / 8); i++) {
                proc_stack[sup.pid][i] = 0;
        }
        for (int i = 0; i < N_CAPS; i++) {
                CapRevoke(&proc->cap_table[i]);
                CapDelete(&proc->cap_table[i]);
        }
        /* Sets initial capability */
        /* TODO: Unload PMP cap in supervisor */
        CapMove(&proc->cap_table[0], cap_pmp);
        /* TODO: Load PMP cap in supervisor */
        current->args[0] = 1;
}

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

void SyscallRevoke(uintptr_t cid) {
        Cap *cap = &current->cap_table[cid % N_CAPS];
        if (cap_is_deleted(cap))
                current->args[0] = 0;
        CapRevoke(cap);
        current->args[0] = 1;
}

void SyscallDelete(uintptr_t cid) {
        Cap *cap = &current->cap_table[cid % N_CAPS];
        current->args[0] = CapDelete(cap);
}

void SyscallSlice(uintptr_t cid_src, uintptr_t cid_dest, uint64_t field0,
                  uint64_t field1) {
        Cap *cap_dest = &current->cap_table[cid_dest % N_CAPS];
        Cap *cap_src = &current->cap_table[cid_src % N_CAPS];
        if (cap_is_deleted(cap_dest)) {
                current->args[0] = 1;
                return;
        }
        cap_dest->field0 = field0;
        cap_dest->field1 = field1;
        if (!cap_is_valid(cap_dest) || !cap_is_child(cap_src, cap_dest)) {
                current->args[0] = 1;
                return;
        }
        /* If parent has a child, error. */
        Cap *cap_next = cap_src->next;
        if (cap_next != NULL && cap_is_child(cap_src, cap_next)) {
                current->args[0] = 1;
                return;
        }
        /* Validate slices */
        current->args[0] = CapInsert(cap_src, cap_dest);
}

void SyscallSplit(uintptr_t cid_src, uintptr_t cid_dest0, uint64_t field00,
                  uint64_t field01, uintptr_t cid_dest1, uint64_t field10,
                  uint64_t field11) {
        Cap *cap_src = &current->cap_table[cid_src % N_CAPS];
        Cap *cap_dest0 = &current->cap_table[cid_dest0 % N_CAPS];
        Cap *cap_dest1 = &current->cap_table[cid_dest1 % N_CAPS];
        if (!cap_is_deleted(cap_dest0) || !cap_is_deleted(cap_dest1)) {
                current->args[0] = 0;
                return;
        }
        cap_dest0->field0 = field00;
        cap_dest0->field1 = field01;
        cap_dest1->field0 = field10;
        cap_dest1->field1 = field11;
        Cap *next = cap_src->next;
        /* Validate slices */
        if (!(next != NULL && cap_is_child(cap_src, next)) &&
            cap_is_child(cap_src, cap_dest0) &&
            cap_is_child(cap_src, cap_dest1)) {
                if (cap_is_type(cap_dest0, CAP_MEMORY_SLICE)) {
                        if (cap_is_type(cap_dest1, CAP_MEMORY_SLICE) &&
                            cap_is_intersect_ms_ms(
                                cap_get_memory_slice(cap_dest0),
                                cap_get_memory_slice(cap_dest1))) {
                                current->args[0] = 4;
                                return;
                        } else if (cap_is_intersect_ms_pe(
                                       cap_get_memory_slice(cap_dest0),
                                       cap_get_pmp_entry(cap_dest1))) {
                                current->args[0] = 4;
                                return;
                        }
                } else if (cap_is_type(cap_dest0, CAP_PMP_ENTRY)) {
                        if (cap_is_type(cap_dest1, CAP_MEMORY_SLICE) &&
                            cap_is_intersect_ms_pe(
                                cap_get_memory_slice(cap_dest1),
                                cap_get_pmp_entry(cap_dest0))) {
                                current->args[0] = 4;
                                return;
                        } else if (cap_is_intersect_ms_pe(
                                       cap_get_memory_slice(cap_dest0),
                                       cap_get_pmp_entry(cap_dest1))) {
                                current->args[0] = 4;
                                return;
                        }
                } else if (cap_is_type(cap_dest0, CAP_TIME_SLICE) &&
                           cap_is_type(cap_dest1, CAP_TIME_SLICE) &&
                           cap_is_intersect_ts_ts(
                               cap_get_time_slice(cap_dest0),
                               cap_get_time_slice(cap_dest1))) {
                        current->args[0] = 4;
                        return;
                }
                if (CapInsert(cap_src, cap_dest0) &&
                    CapInsert(cap_src, cap_dest1)) {
                        current->args[0] = 0;
                } else {
                        current->args[0] = 3;
                }
        }
}
