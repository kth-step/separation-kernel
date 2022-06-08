// See LICENSE file for copyright and license details.
#include "cap.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "proc.h"
#include "sched.h"

/** Capability table */
CapNode cap_tables[N_PROC][N_CAPS];

/* Make one sentinel node per core, one for memory, one for channels and one for
 * supervisor capabilies */
#define N_SENTINELS (N_CORES + 4)
CapNode sentinels[N_SENTINELS];

volatile int ep_receiver[N_CHANNELS];
/*
 * Tries to delete node curr.
 * Assumption: prev = NULL or is_marked(prev->prev)
 */
static inline bool cap_delete(CapNode *prev, CapNode *curr) {
        /* Mark the curr node if it has the correct prev.
         * The CAS is neccessary to avoid the ABA problem.
         */
        if (!__sync_bool_compare_and_swap(&curr->prev, prev, NULL))
                return false;
        CapNode *next = curr->next;
        if (!__sync_bool_compare_and_swap(&next->prev, curr, prev)) {
                curr->prev = prev;
                return false;
        }
        /* TODO: Figure out the ordering of operations */
        curr->next = NULL;
        prev->next = next;
        curr->word0 = 0;
        curr->word1 = 0;
        return true;
}

/*
 * Tries to add child to parent.
 * Returns 1 if successful, otherwise 0.
 * Assumption: parent != NULL and is_marked(parent->prev)
 */
static inline bool cap_append(const Cap cap, CapNode *node, CapNode *prev) {
        CapNode *next = prev->next;
        if (__sync_bool_compare_and_swap(&next->prev, prev, node)) {
                /* TODO: Figure out the ordering of operations */
                prev->next = node;
                node->word0 = cap.word0;
                node->word1 = cap.word1;
                node->next = next;
                node->prev = prev;
                return true;
        }
        return false;
}

bool CapDelete(CapNode *curr) {
        while (!cap_is_deleted(curr)) {
                CapNode *prev = curr->prev;
                if (prev == NULL)
                        continue;
                if (cap_delete(prev, curr))
                        return true;
        }
        return false;
}

bool CapRevoke(CapNode *curr) {
        const Cap parent = cn_get(curr);
        int counter = 0;
        while (!cap_is_deleted(curr)) {
                CapNode *next = curr->next;
                Cap child = cn_get(next);
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
bool CapInsert(const Cap cap, CapNode *node, CapNode *prev) {
        /* Child node must be empty */
        if (!cap_is_deleted(node))
                return false;
        /* While parent is alive, attempt to insert */
        while (!cap_is_deleted(prev)) {
                if (cap_append(cap, node, prev))
                        return true;
        }
        return false;
}

/**
 * Moves the capability in src to dest.
 * Uses a CapInsert followed by CapDelete.
 */
bool CapMove(CapNode *dest, CapNode *src) {
        if (cap_is_deleted(src) || !cap_is_deleted(dest))
                return false;
        Cap cap = cn_get(src);
        return CapInsert(cap, dest, src) && CapDelete(src);
}

bool CapInterprocessMove(CapNode *dest, CapNode *src, int pid_dest, int pid_src) {
        return false;
}

CapNode *CapInitSentinel(void) {
        static int i = 0;
        if (i >= N_SENTINELS)
                return NULL;
        CapNode *sentinel = &sentinels[i++];
        sentinel->next = sentinel;
        sentinel->prev = sentinel;
        sentinel->word0 = 0;
        sentinel->word1 = 0;
        /* Return the head of the sentinel node */
        /* We append capbilities to this head */
        return sentinel;
}
