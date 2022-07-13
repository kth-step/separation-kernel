// See LICENSE file for copyright and license details.
#pragma once

#include "kassert.h"
#include "config.h"
#include "types.h"

#define NULL_CAP ((Cap){0, 0})

typedef struct cap Cap;
typedef struct cap_node CapNode;

struct cap {
        uint64_t word0, word1;
};

struct cap_node {
        CapNode *prev, *next;
        Cap cap;
};

extern CapNode cap_tables[N_PROC][N_CAPS];
extern volatile int ep_receiver[N_CHANNELS];

/* Delete all children of node */
void CapRevoke(CapNode *node);
/* Delete node */
bool CapDelete(CapNode *node);
/* Move node in src to dest */
bool CapMove(CapNode *src, CapNode *dest);
/* Insert node after parent, if insertion is successful, set the data to cap */
bool CapInsert(const Cap cap, CapNode *node, CapNode *parent);
/* Updates the cap in the node */
bool CapUpdate(const Cap cap, CapNode *node);
/* Make a sentinel node. */
CapNode *CapInitSentinel(void);

static inline bool cap_node_is_deleted(const CapNode *cn);
static inline Cap cap_node_get_cap(const CapNode *cn);

/* Check if a node has been deleted */
bool cap_node_is_deleted(const CapNode *cn) {
        // Check if prev is NULL ?
        return cn->next == NULL;
}

Cap cap_node_get_cap(const CapNode *cn) {
        Cap cap = cn->cap;
        __sync_synchronize();
        if (cap_node_is_deleted(cn)) {
                return NULL_CAP;
        }
        return cap;
}
