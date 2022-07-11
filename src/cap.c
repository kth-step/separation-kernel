// See LICENSE file for copyright and license details.
#include "cap.h"

#include "proc.h"
#include "sched.h"

/** Capability table */
CapNode cap_tables[N_PROC][N_CAPS];

/* Make one sentinel node per core, one for memory, one for channels and one for
 * supervisor capabilies */
#define N_SENTINELS (N_CORES + 4)

int n_sentinels = 0;
CapNode sentinels[N_SENTINELS];

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
        curr->cap = NULL_CAP;
        return true;
}

/*
 * Tries to add child to parent.
 * Returns 1 if successful, otherwise 0.
 * Assumption: parent != NULL and is_marked(parent->prev)
 */
static inline bool cap_insert(const Cap cap, CapNode *node, CapNode *prev) {
        CapNode *next = prev->next;
        if (__sync_bool_compare_and_swap(&next->prev, prev, node)) {
                /* TODO: Figure out the ordering of operations */
                prev->next = node;
                node->cap = cap;
                node->next = next;
                node->prev = prev;
                return true;
        }
        return false;
}

bool CapDelete(CapNode *curr) {
        while (!cn_is_deleted(curr)) {
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
        while (!cn_is_deleted(curr)) {
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
        if (!cn_is_deleted(node))
                return false;
        /* While parent is alive, attempt to insert */
        while (!cn_is_deleted(prev)) {
                if (cap_insert(cap, node, prev))
                        return true;
        }
        return false;
}

bool CapUpdate(const Cap cap, CapNode *node) {
        CapNode *prev;
        while ((prev = node->prev)) {
                if (!__sync_bool_compare_and_swap(&node->prev, prev, NULL))
                        continue;
                node->cap = cap;
                __sync_synchronize();
                node->prev = prev;
                return true;
        }
        return false;
}

/**
 * Moves the capability in src to dest.
 * Uses a CapInsert followed by CapDelete.
 */
bool CapMove(CapNode *dest, CapNode *src) {
        const Cap cap = cn_get(src);
        if (cn_is_deleted(src) || !cn_is_deleted(dest))
                return false;
        return CapInsert(cap, dest, src) && CapDelete(src);
}

CapNode *CapInitSentinel(void) {
        kassert(n_sentinels < N_SENTINELS);
        CapNode *sentinel = &sentinels[n_sentinels++];
        sentinel->next = sentinel;
        sentinel->prev = sentinel;
        sentinel->cap = NULL_CAP;
        /* Return the head of the sentinel node */
        /* We append capbilities to this head */
        return sentinel;
}
