// See LICENSE file for copyright and license details.
#include "cap_node.h"

#include "preemption.h"
#include "proc.h"
#include "sched.h"

/** Capability table */
cap_node_t cap_tables[N_PROC][N_CAPS];

/*
 * Tries to delete node curr.
 * Assumption: prev = NULL or is_marked(prev->prev)
 */
bool cap_node_try_delete(cap_node_t* node, cap_node_t* prev)
{
    /* Mark the curr node if it has the correct prev.
     * The CAS is neccessary to avoid the ABA problem.
     */
    if (!__sync_bool_compare_and_swap(&node->prev, prev, NULL))
        return false;
    cap_node_t* next = node->next;
    if (!__sync_bool_compare_and_swap(&next->prev, node, prev)) {
        node->prev = prev;
        return false;
    }
    /* TODO: Figure out the ordering of operations */
    prev->next = next;
    node->next = NULL;
    __sync_synchronize();
    node->cap = NULL_CAP;
    return true;
}

/*
 * Tries to add child to parent.
 * Returns 1 if successful, otherwise 0.
 * Assumption: parent != NULL and is_marked(parent->prev)
 */
bool cap_node_try_insert(cap_t cap, cap_node_t* node, cap_node_t* prev)
{
    cap_node_t* next = prev->next;
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

bool cap_node_delete(cap_node_t* node)
{
    while (!cap_node_is_deleted(node)) {
        cap_node_t* prev = node->prev;
        if (prev == NULL)
            continue;
        if (cap_node_try_delete(node, prev))
            return true;
    }
    return false;
}

void cap_node_revoke(cap_node_t* node)
{
    cap_t parent = cap_node_get_cap(node);
    while (!cap_node_is_deleted(node)) {
        cap_node_t* next = node->next;
        cap_t child = cap_node_get_cap(next);
        if (!cap_is_child(parent, child))
            break;
        uint64_t tmp = preemption_disable();
        cap_node_try_delete(next, node);
        preemption_restore(tmp);
    }
}

/**
 * Insert a child capability after the parent
 * only if the parent is not deleted.
 */
bool cap_node_insert(cap_t cap, cap_node_t* node, cap_node_t* prev)
{
    /* Child node must be empty */
    if (!cap_node_is_deleted(node))
        return false;
    /* While parent is alive, attempt to insert */
    while (!cap_node_is_deleted(prev)) {
        if (cap_node_try_insert(cap, node, prev))
            return true;
    }
    return false;
}

bool cap_node_update(cap_t cap, cap_node_t* node)
{
    cap_node_t* prev;
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
bool cap_node_move(cap_node_t* src, cap_node_t* dest)
{
    return cap_node_insert(cap_node_get_cap(src), dest, src) && cap_node_delete(src);
}
