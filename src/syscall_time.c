// See LICENSE file for copyright and license details.
#include "syscall_time.h"

#include "preemption.h"
#include "s3k_consts.h"
#include "syscall_time.h"

void syscall_time_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);

        if (!cap_can_derive_time(cap, newcap))
                trap_syscall_exit(S3K_ILLEGAL_DERIVATION);
        cap_time_set_free(&cap, cap_time_get_end(newcap));

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t depth = cap_time_get_depth(cap);

        uint64_t newbegin = cap_time_get_begin(newcap);
        uint64_t newend = cap_time_get_end(newcap);
        uint64_t newdepth = cap_time_get_depth(newcap);

        preemption_disable();
        cap_node_update(cap, cn);
        cap_node_insert(newcap, newcn, cn);
        sched_update(newcn, hartid, newbegin, newend, depth, current->pid, newdepth);
        trap_syscall_exit2(S3K_OK);
}

void syscall_time_revoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);

        /* Special revoke routine */
        cap_node_revoke(cn, cap, cap_is_child_time);

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t free = cap_time_get_free(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t depth = cap_time_get_depth(cap);
        cap_time_set_free(&cap, begin);

        preemption_disable();
        sched_update(cn, hartid, begin, free, 0xFF, current->pid, depth);
        cap_node_update(cap, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_time_delete_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);
        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t depth = cap_time_get_depth(cap);
        preemption_disable();
        sched_update(cn, hartid, begin, end, depth, 0xFF, 0xFF);
        cap_node_delete(cn);
        trap_syscall_exit2(S3K_OK);
}
