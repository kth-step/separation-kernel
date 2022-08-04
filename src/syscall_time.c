#include "syscall_time.h"

#include "preemption.h"
#include "s3k_consts.h"

void syscall_time_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);
        kassert(regs == &current->regs);

        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
        cap_t newcap = (cap_t){regs->a2, regs->a3};

        if (!cap_node_is_deleted(newcn)) {
                preemption_disable();
                regs->a0 = S3K_COLLISION;
                regs->pc += 4;
        } else if (!cap_can_derive(cap, newcap)) {
                preemption_disable();
                regs->a0 = S3K_ILLEGAL_DERIVATION;
                regs->pc += 4;
        } else {
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
                regs->a0 = S3K_OK;
                regs->pc += 4;
        }
}

void syscall_time_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);
        kassert(regs == &current->regs);

        /* Special revoke routine */
        cap_node_revoke_time(cn, cap);

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t free = cap_time_get_free(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t depth = cap_time_get_depth(cap);
        cap_time_set_free(&cap, begin);

        preemption_disable();
        sched_update(cn, hartid, begin, free, 0xFF, current->pid, depth);
        cap_node_update(cap, cn);
        regs->a0 = S3K_OK;
        regs->pc += 4;
}

void syscall_time_delete_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);
        kassert(regs == &current->regs);
        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t depth = cap_time_get_depth(cap);
        preemption_disable();
        sched_update(cn, hartid, begin, end, depth, 0xFF, 0xFF);
        cap_node_delete(cn);
        regs->a0 = S3K_OK;
        regs->pc += 4;
}
