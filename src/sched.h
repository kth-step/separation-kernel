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
bool SchedRevoke(const Cap cap, CapNode *cn);

/* Updates scheduling according to time capability cap, check capability node cn
 * for invalidations of the capability. If cn->prev == NULL, break function.*/
bool SchedUpdate(const Cap cap, const Cap new_cap, CapNode *cn);

/* Delete scheduling according to time capability cap, check capability node cn
 * for invalidations of the capability. If cn->prev == NULL, break function.*/
bool SchedDelete(const Cap cap, CapNode *cn);

static inline bool SchedEnablePreemption(void) {
        asm volatile ("csrs mstatus,8");
        return true;
}

static inline bool SchedDisablePreemption(void) {
        asm volatile ("csrc mstatus,8");
        return true;
}
