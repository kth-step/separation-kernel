// See LICENSE file for copyright and license details.
#pragma once

#include "config.h"
#include "proc.h"
#include "platform.h"
#include "types.h"
#include "cap.h"

void Sched(void);

/* Revoke scheduling according to time capability cap, check capability node cn for
 * invalidations of the capability. If cn->prev == NULL, break function. */
bool SchedRevoke(const Cap cap, CapNode *cn);

/* Updates scheduling according to time capability cap, check capability node cn for
 * invalidations of the capability. If cn->prev == NULL, break function.*/
bool SchedUpdate(const Cap cap, const Cap old_cap, CapNode *cn);

/* Delete scheduling according to time capability cap, check capability node cn for
 * invalidations of the capability. If cn->prev == NULL, break function.*/
bool SchedDelete(const Cap cap, CapNode *cn);
 
