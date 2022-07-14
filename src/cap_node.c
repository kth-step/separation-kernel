// See LICENSE file for copyright and license details.
#include "cap_node.h"

#include "preemption.h"
#include "proc.h"
#include "sched.h"

/** Capability table */
struct cap_node cap_tables[N_PROC][N_CAPS];

/* Make one sentinel node per core, one for memory, one for channels and one for
 * supervisor capabilies */
#define N_SENTINELS (N_CORES + 4)

int n_sentinels = 0;
struct cap_node sentinels[N_SENTINELS];

/*
 * Tries to delete node curr.
 * Assumption: prev = NULL or is_marked(prev->prev)
 */
static inline bool cap_delete(struct cap_node *prev, struct cap_node *curr) {
        /* Mark the curr node if it has the correct prev.
         * The CAS is neccessary to avoid the ABA problem.
         */
        if (!__sync_bool_compare_and_swap(&curr->prev, prev, NULL))
                return false;
        struct cap_node *next = curr->next;
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
static inline bool cap_insert(struct cap cap, struct cap_node *node,
                              struct cap_node *prev) {
        struct cap_node *next = prev->next;
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

bool CapDelete(struct cap_node *curr) {
        while (!cap_node_is_deleted(curr)) {
                struct cap_node *prev = curr->prev;
                if (prev == NULL)
                        continue;
                if (cap_delete(prev, curr))
                        return true;
        }
        return false;
}

void CapRevoke(struct cap_node *curr) {
        struct cap parent = cap_node_get_cap(curr);
        while (!cap_node_is_deleted(curr)) {
                uint64_t tmp = preemption_disable();
                struct cap_node *next = curr->next;
                struct cap child = cap_node_get_cap(next);
                if (!cap_is_child(parent, child))
                        break;
                cap_delete(curr, next);
                preemption_restore(tmp);
        }
}

/**
 * Insert a child capability after the parent
 * only if the parent is not deleted.
 */
bool CapInsert(struct cap cap, struct cap_node *node, struct cap_node *prev) {
        /* Child node must be empty */
        if (!cap_node_is_deleted(node))
                return false;
        /* While parent is alive, attempt to insert */
        while (!cap_node_is_deleted(prev)) {
                if (cap_insert(cap, node, prev))
                        return true;
        }
        return false;
}

bool CapUpdate(struct cap cap, struct cap_node *node) {
        struct cap_node *prev;
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
bool CapMove(struct cap_node *dest, struct cap_node *src) {
        struct cap cap = cap_node_get_cap(src);
        if (cap_node_is_deleted(src) || !cap_node_is_deleted(dest))
                return false;
        return CapInsert(cap, dest, src) && CapDelete(src);
}

struct cap_node *CapInitSentinel(void) {
        kassert(n_sentinels < N_SENTINELS);
        struct cap_node *sentinel = &sentinels[n_sentinels++];
        sentinel->next = sentinel;
        sentinel->prev = sentinel;
        sentinel->cap = NULL_CAP;
        /* Return the head of the sentinel node */
        /* We append capbilities to this head */
        return sentinel;
}
