#pragma once
#include "proc.h"
#include "sched.h"

void syscall_handle_time(registers_t* regs, cap_node_t* cn, cap_t cap);

static inline uint64_t time_interprocess_move(cap_t cap, proc_t* psrc, proc_t* pdest, cap_node_t* cnsrc, cap_node_t* cndest)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t depth = cap_time_get_depth(cap);

        if (!cap_node_is_deleted(cndest))
                return S3K_COLLISION;
        return cap_node_move(cnsrc, cndest) && sched_update(cndest, hartid, begin, end, depth, pdest->pid, depth) ? S3K_OK : S3K_EMPTY;
}
