// See LICENSE file for copyright and license details.
#include "cap.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "atomic.h"
#include "sched.h"
#include "cap_util.h"

/** Capability table */
Cap cap_tables[N_PROC][N_CAPS];

static inline bool cap_try_mark(Cap *node) {
        return !is_marked(amoor(&node->prev, mark_bit));
}

static inline void cap_unmark(Cap *node) {
        amoand(&node->prev, ~mark_bit);
}

/*
 * Tries to delete node curr.
 * Assumption: prev = NULL or is_marked(prev->prev)
 */
static inline bool cap_delete(Cap *prev, Cap *curr) {
        /* Mark the curr node if it has the correct prev.
         * The CAS is neccessary to avoid the ABA problem.
         */
        if (!compare_and_swap(&curr->prev, prev, mark(prev)))
                return false;
        Cap *next = curr->next;
        if (next != NULL && !compare_and_swap(&next->prev, curr, prev)) {
                cap_unmark(curr);
                return false;
        }
        /* Delete has now succeeded,
         * prev->next is now inconsistent, just fix it.
         */
        curr->next = NULL;
        if (prev)
                prev->next = next;
        return true;
}

/*
 * Tries to add child to parent.
 * Returns 1 if successful, otherwise 0.
 * Assumption: parent != NULL and is_marked(parent->prev)
 */
static inline bool cap_append(Cap *node, Cap *prev) {
        Cap *next = prev->next;
        if (next == NULL || compare_and_swap(&next->prev, prev, node)) {
                fence(w, w);
                node->next = next;
                fence(w, w);
                prev->next = node;
                fence(w, w);
                node->prev = prev;
                return true;
        }
        return false;
}

bool CapDelete(Cap *curr) {
        while (1) {
                Cap *prev = unmark(curr->prev);
                if (prev != NULL && !cap_try_mark(prev))
                        continue;
                bool succ = cap_delete(prev, curr);
                if (prev != NULL)
                        cap_unmark(prev);
                if (succ || cap_is_deleted(curr))
                        return succ;
        }
}

/*
void cap_revoke_ms(Cap *curr) {
        Cap *next = curr->next;
        CapMemorySlice ms = cap_get_memory_slice(curr);
        while (!cap_is_deleted(curr) && next) {
                sched_preemption();
                // Get the previous node
                // Lock curr
                if (!cap_try_mark(curr))
                        continue;
                cap_delete(curr, next);
                // Unlock curr
                cap_unmark(curr);
                next = curr->next;
        }
}
void cap_revoke_ts(Cap *curr) {
        Cap *next = curr->next;
        CapTimeSlice ts = cap_get_time_slice(curr);
        while (!cap_is_deleted(curr) && next) {
                sched_preemption();
                // Lock curr
                if (!cap_try_mark(curr))
                        continue;
                cap_delete(curr, next);
                // Unlock curr
                cap_unmark(curr);
                next = curr->next;
        }
}
*/

void CapRevoke(Cap *curr) {
}

/**
 * Insert a child capability after the parent
 * only if the parent is not deleted.
 */
bool CapAppend(Cap *node, Cap *prev) {
        /* Child node must be empty */
        if (!cap_is_deleted(node))
                return false;
        bool succ = false;
        /* While parent is alive, attempt to insert */
        while (!succ && !cap_is_deleted(prev)) {
                if (!cap_try_mark(prev))
                        continue;
                succ = cap_append(node, prev);
                cap_unmark(prev);
        }
        return succ;
}

/**
 * Moves the capability in src to dest.
 * Uses a CapInsert followed by CapDelete.
 */
bool CapMove(Cap *dest, Cap *src) {
        if (cap_is_deleted(src))
                return false;
        dest->data[0] = src->data[0];
        dest->data[1] = src->data[1];
        if (CapAppend(src, dest)) {
                // First insert the copy after the parent
                // Then delete the parent.
                CapDelete(src);
                return true;
        }
        return false;
}

uint64_t CapSliceTS(Cap *src, Cap *dest, uint64_t data[2]) {
        return -1;
}

uint64_t CapSplitTS(Cap *src, Cap *dest0, Cap *dest1, uint64_t data0[2],
                    uint64_t data1[2]) {
        return -1;
}

uint64_t CapSliceMS(Cap *src, Cap *dest, uint64_t data[2]) {
        return -1;
}

uint64_t CapSplitMS(Cap *src, Cap *dest0, Cap *dest1, uint64_t data0[2],
                    uint64_t data1[2]) {
        return -1;
}
