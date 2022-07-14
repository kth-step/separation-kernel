// See LICENSE file for copyright and license details.
#pragma once

#include "cap.h"
#include "config.h"
#include "kassert.h"
#include "types.h"

struct cap_node {
        struct cap_node *prev, *next;
        struct cap cap;
};

extern struct cap_node cap_tables[N_PROC][N_CAPS];
extern volatile int ep_receiver[N_CHANNELS];

/* Delete all children of node */
void CapRevoke(struct cap_node *node);
/* Delete node */
bool CapDelete(struct cap_node *node);
/* Move node in src to dest */
bool CapMove(struct cap_node *src, struct cap_node *dest);
/* Insert node after parent, if insertion is successful, set the data to cap */
bool CapInsert(struct cap cap, struct cap_node *node, struct cap_node *parent);
/* Updates the cap in the node */
bool CapUpdate(struct cap cap, struct cap_node *node);
/* Make a sentinel node. */
struct cap_node *CapInitSentinel(void);

static inline bool cap_node_is_deleted(struct cap_node *cn);
static inline struct cap cap_node_get_cap(struct cap_node *cn);

/* Check if a node has been deleted */
bool cap_node_is_deleted(struct cap_node *cn) {
        // Check if prev is NULL ?
        return cn->next == NULL;
}

struct cap cap_node_get_cap(struct cap_node *cn) {
        struct cap cap = cn->cap;
        __sync_synchronize();
        if (cap_node_is_deleted(cn)) {
                return ((struct cap){0, 0});
        }
        return cap;
}
