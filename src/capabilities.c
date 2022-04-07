// See LICENSE file for copyright and license details.
#include "capabilities.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "atomic.h"

static inline bool cap_try_mark(Capability *node) {
        return !is_marked(amoor(&node->prev, mark_bit));
}

static inline void cap_unmark(Capability *node) {
        amoand(&node->prev, ~mark_bit);
}

/*
 * Tries to delete node curr.
 * Assumption: prev = NULL or is_marked(prev->prev)
 */
static inline void cap_delete(Capability *prev, Capability *curr) {
        /* Mark the curr node if it has the correct prev.
         * The CAS is neccessary to avoid the ABA problem.
         */
        if (!compare_and_swap(&curr->prev, prev, mark(prev)))
                return;
        Capability *next = curr->next;
        if (next == NULL || compare_and_swap(&next->prev, curr, prev)) {
                /* Delete has now succeeded,
                 * prev->next is now inconsistent, just fix it.
                 */
                curr->field0 = 0;
                curr->field1 = 0;
                fence(w, w);
                curr->next = NULL;
                if (prev)
                        prev->next = next;
        } else {
                /* CAS failed so abort deletion */
                cap_unmark(curr);
        }
}

/*
 * Tries to add child to parent.
 * Returns 1 if successful, otherwise 0.
 * Assumption: parent != NULL and is_marked(parent->prev)
 */
static inline bool cap_insert(Capability *parent, Capability *child) {
        Capability *next = parent->next;
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

void CapDelete(Capability *curr) {
        do {
                // TODO: Check preemption
                // Get the previous node
                Capability *prev = unmark(curr->prev);
                if (prev != NULL && !cap_try_mark(prev))
                        continue;
                cap_delete(prev, curr);
                if (prev != NULL)
                        cap_unmark(prev);
        } while (!cap_is_deleted(curr));
        return;
}

void cap_revoke_ms(Capability *curr) {
        Capability *next = curr->next;
        CapMemorySlice ms = cap_get_memory_slice(curr);
        while (!cap_is_deleted(curr) && next && cap_is_child_ms(ms, next)) {
                // TODO: Check preemption
                // Lock curr
                if (!cap_try_mark(curr))
                        continue;
                cap_delete(curr, next);
                // Unlock curr
                cap_unmark(curr);
                next = curr->next;
        }
}

void cap_revoke_ts(Capability *curr) {
        Capability *next = curr->next;
        CapTimeSlice ts = cap_get_time_slice(curr);
        while (!cap_is_deleted(curr) && next &&
               cap_is_child_ts_ts(ts, cap_get_time_slice(next))) {
                // TODO: Check preemption
                // Lock curr
                if (!cap_try_mark(curr))
                        continue;
                cap_delete(curr, next);
                // Unlock curr
                cap_unmark(curr);
                next = curr->next;
        }
}

void CapRevoke(Capability *curr) {
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
int CapInsert(Capability *parent, Capability *child) {
        if (!cap_is_deleted(child))
                return false;
        while (!cap_is_deleted(parent) && cap_is_deleted(child)) {
                // TODO: Check preemption
                if (!cap_try_mark(parent))
                        continue;
                if (cap_insert(parent, child)) {
                        cap_unmark(parent);
                        return true;
                }
                cap_unmark(parent);
        }
        return false;
}

/**
 * Moves the capability in src to dest.
 * Uses a CapInsert followed by CapDelete.
 */
int CapMove(Capability *dest, Capability *src) {
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
