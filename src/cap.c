// See LICENSE file for copyright and license details.
#include "cap.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "atomic.h"
#include "cap_util.h"
#include "sched.h"

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
                if (!cap_try_mark(curr))
                        continue;
                cap_delete(curr, next);
                cap_unmark(curr);
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
                if (!cap_try_mark(curr))
                        continue;
                cap_delete(curr, next);
                cap_unmark(curr);
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
        if (cap_is_deleted(src) || !cap_is_deleted(dest))
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
