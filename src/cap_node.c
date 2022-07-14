// See LICENSE file for copyright and license details.
#include "cap_node.h"

#include "preemption.h"
#include "proc.h"
#include "sched.h"

/** Capability table */
struct cap_node cap_tables[N_PROC][N_CAPS];

/*
 * Tries to delete node curr.
 * Assumption: prev = NULL or is_marked(prev->prev)
 */
static inline bool __cap_node_delete(struct cap_node* prev, struct cap_node* curr)
{
        /* Mark the curr node if it has the correct prev.
     * The CAS is neccessary to avoid the ABA problem.
     */
        if (!__sync_bool_compare_and_swap(&curr->prev, prev, NULL))
                return false;
        struct cap_node* next = curr->next;
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
static inline bool __cap_node_insert(struct cap cap, struct cap_node* node, struct cap_node* prev)
{
        struct cap_node* next = prev->next;
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

bool cap_node_delete(struct cap_node* curr)
{
        while (!cap_node_is_deleted(curr)) {
                struct cap_node* prev = curr->prev;
                if (prev == NULL)
                        continue;
                if (__cap_node_delete(prev, curr))
                        return true;
        }
        return false;
}

void cap_node_revoke(struct cap_node* curr)
{
        struct cap parent = cap_node_get_cap(curr);
        while (!cap_node_is_deleted(curr)) {
                uint64_t tmp = preemption_disable();
                struct cap_node* next = curr->next;
                struct cap child = cap_node_get_cap(next);
                if (!cap_is_child(parent, child))
                        break;
                __cap_node_delete(curr, next);
                preemption_restore(tmp);
        }
}

/**
 * Insert a child capability after the parent
 * only if the parent is not deleted.
 */
bool cap_node_insert(struct cap cap, struct cap_node* node, struct cap_node* prev)
{
        /* Child node must be empty */
        if (!cap_node_is_deleted(node))
                return false;
        /* While parent is alive, attempt to insert */
        while (!cap_node_is_deleted(prev)) {
                if (__cap_node_insert(cap, node, prev))
                        return true;
        }
        return false;
}

bool cap_node_update(struct cap cap, struct cap_node* node)
{
        struct cap_node* prev;
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
bool cap_node_move(struct cap_node* dest, struct cap_node* src)
{
        struct cap cap = cap_node_get_cap(src);
        if (cap_node_is_deleted(src) || !cap_node_is_deleted(dest))
                return false;
        return cap_node_insert(cap, dest, src) && cap_node_delete(src);
}
