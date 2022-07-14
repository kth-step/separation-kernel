// See LICENSE file for copyright and license details.
#pragma once

#include "cap.h"
#include "config.h"
#include "platform.h"
#include "proc.h"
#include "types.h"

void sched(void);

/* Revoke scheduling according to time capability cap, check capability node cn
 * for invalidations of the capability. If cn->prev == NULL, break function. */
bool sched_revoke(struct cap cap, struct cap_node* cn);

/* Updates scheduling according to time capability cap, check capability node
 * cn for invalidations of the capability. If cn->prev == NULL, break
 * function.*/
bool sched_update(struct cap cap, struct cap new_cap, struct cap_node* cn);

/* Delete scheduling according to time capability cap, check capability node cn
 * for invalidations of the capability. If cn->prev == NULL, break function.*/
bool sched_delete(struct cap cap, struct cap_node* cn);
