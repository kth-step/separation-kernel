#include "syscall_time.h"

#include "preemption.h"
#include "s3k_consts.h"

void syscall_time_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);

        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
        cap_t newcap = (cap_t){regs->a2, regs->a3};

        if (!cap_node_is_deleted(newcn)) {
                preemption_disable();
                regs->a0 = S3K_COLLISION;
                regs->pc += 4;
                preemption_enable();
                return;
        }

        if (!cap_can_derive(cap, newcap)) {
                preemption_disable();
                regs->a0 = S3K_ILLEGAL_DERIVATION;
                regs->pc += 4;
                preemption_enable();
                return;
        }

        kassert(cap_get_type(newcap) == CAP_TYPE_TIME);

        cap_time_set_free(&cap, cap_time_get_end(newcap));

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t depth = cap_time_get_depth(cap);

        uint64_t newbegin = cap_time_get_begin(newcap);
        uint64_t newend = cap_time_get_end(newcap);
        uint64_t newdepth = cap_time_get_depth(newcap);

        preemption_disable();
        if (cap_node_update(cap, cn) && cap_node_insert(newcap, newcn, cn) && sched_update(newcn, hartid, newbegin, newend, depth, current->pid, newdepth)) {
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_ERROR;
        }
        regs->pc += 4;
        preemption_enable();
}

void syscall_time_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t free = cap_time_get_free(cap);
        uint64_t depth = cap_time_get_depth(cap);

        /* Special revoke routine */
        while (1) {
                /* Check that cn has note been deleted */
                cap_node_t* next = cn->next;
                if (next == NULL)
                        break;

                /* Check if next cap is a child */
                cap_t cap_next = cap_node_get_cap(next);
                if (!cap_is_child(cap, cap_next))
                        break;

                /* Disable preemption */
                preemption_disable();
                /* If cap_next is child, it must be a time slice */
                kassert(cap_get_type(cap_next) == CAP_TYPE_TIME);

                /* Get the beginning, end and depth of child slice */
                uint64_t begin_next = cap_time_get_begin(cap_next);
                uint64_t end_next = cap_time_get_end(cap_next);
                uint64_t depth_next = cap_time_get_depth(cap_next);
                /* Try delete the slice */
                if (cap_node_try_delete(next, cn)) {
                        /* If slice is deleted, revoke the associated time */
                        sched_update(cn, hartid, begin_next, end_next, depth_next, current->pid, depth);
                }
                preemption_enable();
        }

        cap_time_set_free(&cap, begin);

        preemption_disable();
        if (sched_update(cn, hartid, begin, free, 0xFF, current->pid, depth) && cap_node_update(cap, cn)) {
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_ERROR;
        }
        regs->pc += 4;
        preemption_enable();
}

void syscall_time_delete_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t depth = cap_time_get_depth(cap);
        preemption_disable();
        if (sched_update(cn, hartid, begin, end, depth, 0xFF, 0xFF) && cap_node_delete(cn)) {
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_ERROR;
        }
        regs->pc += 4;
        preemption_disable();
}
