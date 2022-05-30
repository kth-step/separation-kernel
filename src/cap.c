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
        curr->data[0] = 0;
        curr->data[1] = 0;
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

bool CapRevoke(Cap *curr) {
        const CapUnion parent = cap_get(curr);
        int counter = 0;
        while (!cap_is_deleted(curr)) {
                Cap *next = curr->next;
                CapUnion child = cap_get(next);
                if (!cap_is_child(parent, child))
                        break;
                if (cap_delete(curr, next))
                        counter++;
        }
        return counter > 0;
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

bool CapInterprocessMove(Cap *dest, Cap *src, int pid_dest, int pid_src) {
        return false;
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
