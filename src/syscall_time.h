// See LICENSE file for copyright and license details.
#pragma once
#include "proc.h"
#include "sched.h"
#include "trap.h"

void syscall_time_delete_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_time_revoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_time_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
    __attribute__((noreturn));

static inline uint64_t time_interprocess_move(cap_t cap, proc_t* psrc, proc_t* pdest,
                                              cap_node_t* cnsrc, cap_node_t* cndest)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t depth = cap_time_get_depth(cap);

        if (!cap_node_is_deleted(cndest))
                return S3K_COLLISION;
        cap_node_move(cnsrc, cndest);
        sched_update(cndest, hartid, begin, end, depth, pdest->pid, depth);
        return S3K_OK;
}
