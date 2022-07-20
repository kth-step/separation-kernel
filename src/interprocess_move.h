#pragma once
#include "proc.h"
#include "s3k_consts.h"
#include "syscall_supervisor.h"
#include "syscall_time.h"

static inline uint64_t interprocess_move(proc_t* psrc, proc_t* pdest, uint64_t cidsrc, uint64_t ciddest)
{

    cap_node_t* cnsrc = proc_get_cap_node(psrc, cidsrc);
    cap_node_t* cndest = proc_get_cap_node(pdest, ciddest);

    cap_t cap = cap_node_get_cap(cnsrc);

    switch (cap_get_type(cap)) {
    case CAP_TYPE_TIME:
        return time_interprocess_move(cap, psrc, pdest, cnsrc, cndest);

    case CAP_TYPE_EMPTY:
        return S3K_EMPTY;

    default:
        if (cap_node_is_deleted(cndest))
            return cap_node_move(cnsrc, cndest) ? S3K_OK : S3K_EMPTY;
        return S3K_COLLISION;
    }
}
