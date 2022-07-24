// See LICENSE file for copyright and license details.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cap.h"
#include "config.h"
#include "kassert.h"

typedef struct cap_node cap_node_t;

struct cap_node {
        cap_node_t *prev, *next;
        cap_t cap;
};

extern cap_node_t cap_tables[N_PROC][N_CAPS];

/* Delete node */
bool cap_node_try_delete(cap_node_t* node, cap_node_t* prev);

/* Insert node after parent, if insertion is successful, set the data to cap */
bool cap_node_try_insert(cap_t cap, cap_node_t* node, cap_node_t* parent);

/* Delete all children of node */
void cap_node_revoke(cap_node_t* node);

/* Delete node */
bool cap_node_delete(cap_node_t* node);

/* Move node in src to dest */
bool cap_node_move(cap_node_t* src, cap_node_t* dest);

/* Insert node after parent, if insertion is successful, set the data to cap */
bool cap_node_insert(cap_t cap, cap_node_t* node, cap_node_t* parent);

/* Updates the cap in the node */
bool cap_node_update(cap_t cap, cap_node_t* node);

static inline bool cap_node_is_deleted(cap_node_t* cn);
static inline cap_t cap_node_get_cap(cap_node_t* cn);

/* Check if a node has been deleted */
bool cap_node_is_deleted(cap_node_t* cn)
{
        // Check if prev is NULL ?
        return cn->next == NULL;
}

cap_t cap_node_get_cap(cap_node_t* cn)
{
        cap_t cap = cn->cap;
        __sync_synchronize();
        if (cap_node_is_deleted(cn)) {
                return ((cap_t){0, 0});
        }
        return cap;
}
