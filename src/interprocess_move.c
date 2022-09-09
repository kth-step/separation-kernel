#include "interprocess_move.h"

#include "sched.h"

uint64_t interprocess_move(proc_t* src_proc, uint64_t src_cid, proc_t* dest_proc, uint64_t dest_cid)
{
    cap_node_t* src_node = proc_get_cap_node(src_proc, src_cid);
    cap_node_t* dest_node = proc_get_cap_node(dest_proc, dest_cid);
    cap_t cap = cap_node_get_cap(src_node);

    if (cap_node_is_deleted(src_node))
        return S3K_EMPTY;
    if (!cap_node_is_deleted(dest_node))
        return S3K_COLLISION;

    if (cap_is_type(cap, CAP_TYPE_TIME)) {
        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t free = cap_time_get_free(cap);
        uint64_t end = cap_time_get_end(cap);
        /* Update schedule to give time to receiving process */
        sched_update(src_node, hartid, free, end, dest_proc->pid);
    }

    if (!cap_node_move(cap, src_node, dest_node))
        return S3K_EMPTY;
    return S3K_OK;
}
