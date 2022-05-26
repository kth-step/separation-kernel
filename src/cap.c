// See LICENSE file for copyright and license details.
#include "cap.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "cap_util.h"
#include "proc.h"
#include "sched.h"

/** Capability table */
Cap cap_tables[N_PROC][N_CAPS];

/* Make one sentinel node per core, one for memory, one for channels and one for
 * supervisor capabilies */
#define N_SENTINELS (N_CORES + 4)
Cap sentinels[N_SENTINELS];

volatile int ep_receiver[N_CHANNELS];
/*
 * Tries to delete node curr.
 * Assumption: prev = NULL or is_marked(prev->prev)
 */
static inline bool cap_delete(Cap *prev, Cap *curr) {
        /* Mark the curr node if it has the correct prev.
         * The CAS is neccessary to avoid the ABA problem.
         */
        if (!__sync_bool_compare_and_swap(&curr->prev, prev, NULL))
                return false;
        Cap *next = curr->next;
        if (!__sync_bool_compare_and_swap(&next->prev, curr, prev)) {
                curr->prev = prev;
                return false;
        }
        prev->next = next;
        __sync_synchronize();
        curr->next = NULL;
        return true;
}

/*
 * Tries to add child to parent.
 * Returns 1 if successful, otherwise 0.
 * Assumption: parent != NULL and is_marked(parent->prev)
 */
static inline bool cap_append(Cap *node, Cap *prev) {
        Cap *next = prev->next;
        if (__sync_bool_compare_and_swap(&next->prev, prev, node)) {
                node->next = next;
                prev->next = node;
                __sync_synchronize();
                node->prev = prev;
                return true;
        }
        return false;
}

bool CapDelete(Cap *curr) {
        while (!cap_is_deleted(curr)) {
                Cap *prev = curr->prev;
                if (prev == NULL)
                        continue;
                if (cap_delete(prev, curr))
                        return true;
        }
        return false;
}

void cap_revoke_ms(Cap *curr) {
        CapMemorySlice parent = cap_get_memory_slice(curr);
        if (!parent.valid)
                return;
        do {
                Cap *next = curr->next;
                if (!next)
                        break;
                // TODO: Fix PMP Entry case
                CapMemorySlice child = cap_get_memory_slice(next);
                if (!child.valid)
                        break;
                if (!cap_is_child_ms_ms(parent, child))
                        break;
                cap_delete(curr, next);
        } while (!cap_is_deleted(curr));
}

void cap_revoke_ts(Cap *curr) {
        CapTimeSlice parent = cap_get_time_slice(curr);
        if (!parent.valid)
                return;
        do {
                Cap *next = curr->next;
                if (!next)
                        break;
                CapTimeSlice child = cap_get_time_slice(next);
                if (!child.valid)
                        break;
                if (!cap_is_child_ts_ts(parent, child))
                        break;
                cap_delete(curr, next);
        } while (!cap_is_deleted(curr));
}

void CapRevoke(Cap *curr) {
        CapType type = cap_get_type(curr);
        switch (type) {
                case CAP_MEMORY_SLICE:
                        cap_revoke_ms(curr);
                        break;
                case CAP_TIME_SLICE:
                        cap_revoke_ts(curr);
                        break;
                default:
                        break;
        }
}

/**
 * Insert a child capability after the parent
 * only if the parent is not deleted.
 */
bool CapAppend(Cap *node, Cap *prev) {
        /* Child node must be empty */
        if (!cap_is_deleted(node))
                return false;
        /* While parent is alive, attempt to insert */
        while (!cap_is_deleted(prev)) {
                if (cap_append(node, prev))
                        return true;
        }
        return false;
}

/**
 * Moves the capability in src to dest.
 * Uses a CapInsert followed by CapDelete.
 */
bool CapMove(Cap *dest, Cap *src) {
        if (cap_is_deleted(src) || !cap_is_deleted(dest))
                return false;
        dest->data[0] = src->data[0];
        dest->data[1] = src->data[1];
        return CapAppend(dest, src) && CapDelete(src);
}

bool CapInterprocessMoveTimeSlice(Cap *dest, Cap *src, int pid_dest,
                                  int pid_src) {
        if (!cap_is_deleted(dest) || cap_is_deleted(src))
                return false;
        CapTimeSlice ts = cap_get_time_slice(src);
        if (!ts.valid)
                return false;
        uint8_t tsid = ts.tsid;
        SchedUpdatePidTsid(ts.end, ts.begin, ts.hartid, pid_src, tsid,
                           pid_dest, tsid);
        return CapMove(dest, src);
}

bool CapPmpEntryLoad(int8_t pid, Cap *cap_pmp, int pmp_index) {
        CapPmpEntry pmp = cap_get_pmp_entry(cap_pmp);
        if (!pmp.valid)
                return false;
        return ProcPmpLoad(pid, pmp_index, pmp.addr, pmp.rwx);
}

bool CapPmpEntryUnload(int8_t pid, Cap *cap_pmp) {
        CapPmpEntry pmp = cap_get_pmp_entry(cap_pmp);
        if (!pmp.valid || pmp.index == -1)
                return false;
        return ProcPmpUnload(pid, pmp.index);
}

bool CapInterprocessMove(Cap *dest, Cap *src, int pid_dest, int pid_src) {
        CapType type = cap_get_type(src);
        if (type == CAP_TIME_SLICE) {
                return CapInterprocessMoveTimeSlice(dest, src, pid_dest,
                                                    pid_src);
        } else if (type == CAP_PMP_ENTRY) {
                CapPmpEntryUnload(pid_src, src);
        }
        if (!cap_is_deleted(dest) || cap_is_deleted(src))
                return false;
        return CapMove(dest, src);
}

Cap *CapInitSentinel(void) {
        static int i = 0;
        if (i >= N_SENTINELS)
                return NULL;
        Cap *sentinel = &sentinels[i++];
        sentinel->next = sentinel;
        sentinel->prev = sentinel;
        sentinel->data[0] = 0;
        sentinel->data[1] = 0;
        /* Return the head of the sentinel node */
        /* We append capbilities to this head */
        return sentinel;
}
