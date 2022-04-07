// See LICENSE file for copyright and license details.
#include "capabilities.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "atomic.h"
#include "sched.h"

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
static inline bool cap_insert(Cap *parent, Cap *child) {
        Cap *next = parent->next;
        if (next == NULL || compare_and_swap(&next->prev, parent, child)) {
                fence(w, w);
                child->next = next;
                fence(w, w);
                parent->next = child;
                fence(w, w);
                child->prev = parent;
                return true;
        }
        return false;
}

extern void AsmSwitchToSched();

bool CapDelete(Cap *curr) {
        while (1) {
                Cap *prev = unmark(curr->prev);
                sched_preemption();

                if (prev != NULL && !cap_try_mark(prev))
                        continue;
                bool succ = cap_delete(prev, curr);
                if (prev != NULL)
                        cap_unmark(prev);
                if (succ || cap_is_deleted(curr))
                        return succ;
        }
}

void cap_revoke_ms(Cap *curr) {
        Cap *next = curr->next;
        CapMemorySlice ms = cap_get_memory_slice(curr);
        while (!cap_is_deleted(curr) && next && cap_is_child_ms(ms, next)) {
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
        while (!cap_is_deleted(curr) && next &&
               cap_is_child_ts_ts(ts, cap_get_time_slice(next))) {
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

void CapRevoke(Cap *curr) {
        switch (cap_get_type(curr)) {
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
bool CapInsert(Cap *parent, Cap *child) {
        /* Child node must be empty */
        if (!cap_is_deleted(child))
                return false;
        bool succ = false;
        /* While parent is alive, attempt to insert */
        while (!succ && !cap_is_deleted(parent)) {
                /* Check if timer has been triggered */
                sched_preemption();
                if (cap_try_mark(parent))
                        continue;
                succ = cap_insert(parent, child);
                cap_unmark(parent);
        }
        return succ;
}

/**
 * Moves the capability in src to dest.
 * Uses a CapInsert followed by CapDelete.
 */
bool CapMove(Cap *dest, Cap *src) {
        dest->field0 = src->field0;
        dest->field1 = src->field1;
        if (CapInsert(src, dest)) {
                // First insert the copy after the parent
                // Then delete the parent.
                CapDelete(src);
                return true;
        }
        return false;
}
