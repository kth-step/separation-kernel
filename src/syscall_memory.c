// See LICENSE file for copyright and license details.
#include "syscall_memory.h"

#include "trap.h"

void syscall_memory_revoke_cap(cap_node_t* cn, cap_t cap)
{
        cap_node_revoke(cn, cap, cap_is_child_memory);
        cap_memory_set_free(&cap, cap_memory_get_begin(cap));
        cap_memory_set_pmp(&cap, 0);
        preemption_disable();
        cap_node_update(cap, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_memory_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_MEMORY);

        if (!cap_can_derive_memory(cap, newcap))
                trap_syscall_exit1(S3K_ILLEGAL_DERIVATION);

        if (cap_get_type(newcap) == CAP_TYPE_MEMORY)
                cap_memory_set_free(&cap, cap_memory_get_end(newcap));
        if (cap_get_type(newcap) == CAP_TYPE_PMP)
                cap_memory_set_pmp(&cap, 1);

        preemption_disable();
        cap_node_update(cap, cn);
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}
