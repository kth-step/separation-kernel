// See LICENSE file for copyright and license details.
#include "cap_node.h"

#include "preemption.h"
#include "proc.h"
#include "sched.h"

/** Capability table */
cap_node_t cap_tables[N_PROC][N_CAPS];

static inline bool _cap_node_delete(cap_node_t* node, cap_node_t* prev)
{
        cap_node_t* next;
        if (!compare_and_set(&node->prev, prev, NULL))
                return false;
        do {
                next = node->next;
        } while (!compare_and_set(&next->prev, node, prev));
        prev->next = next;
        return true;
}

bool cap_node_delete(cap_node_t* node)
{
        cap_node_t* prev;
        do {
                prev = node->prev;
                if (prev == NULL)
                        return false;
        } while (!_cap_node_delete(node, prev));
        return true;
}

void cap_node_revoke(cap_node_t* node, cap_t parent)
{
        cap_node_t* next;
        while (node->prev != NULL) {
                next = node->next;
                cap_t child = cap_node_get_cap(next);
                if (!cap_is_child(parent, child))
                        return;
                uint64_t tmp = preemption_disable();
                _cap_node_delete(next, node);
                preemption_restore(tmp);
        }
}

void cap_node_revoke_time(cap_node_t* node, cap_t parent)
{
        kassert(cap_get_type(parent) == CAP_TYPE_TIME);
        uint64_t hartid = cap_time_get_hartid(parent);
        uint64_t depth = cap_time_get_depth(parent);
        cap_node_t* next;
        while (node->prev != NULL) {
                next = node->next;
                cap_t child = cap_node_get_cap(next);
                if (!cap_is_child(parent, child))
                        return;
                uint64_t tmp = preemption_disable();
                if (_cap_node_delete(next, node)) {
                        uint64_t cbegin = cap_time_get_begin(child);
                        uint64_t cend = cap_time_get_end(child);
                        uint64_t cdepth = cap_time_get_depth(child);
                        sched_update(node, hartid, cbegin, cend, cdepth, current->pid, depth);
                }
                preemption_restore(tmp);
        }
}

/**
 * Insert a child capability after the parent
 * only if the parent is not deleted.
 */
bool cap_node_insert(cap_t cap, cap_node_t* node, cap_node_t* prev)
{
        kassert(node->prev == NULL);
        node->cap = cap;
        cap_node_t* next = prev->next;
        while (prev->prev != NULL) {
                node->next = next;
                if (compare_and_set(&next->prev, prev, node)) {
                        prev->next = node;
                        node->prev = prev;
                        return true;
                }
                next = prev->next;
        }
        return false;
}

bool cap_node_move(cap_node_t* src, cap_node_t* dest)
{
        return cap_node_insert(src->cap, dest, src) && cap_node_delete(src);
}

bool cap_node_update(cap_t cap, cap_node_t* node)
{
        node->cap = cap;
        return node->prev != NULL;
}
