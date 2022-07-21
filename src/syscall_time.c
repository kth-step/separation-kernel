#include "syscall_time.h"

#include "preemption.h"
#include "s3k_consts.h"

static inline uint64_t time_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);
        if (!cap_node_is_deleted(newcn))
                return S3K_COLLISION;

        if (!cap_can_derive(cap, newcap))
                return S3K_ILLEGAL_DERIVATION;

        kassert(cap_get_type(newcap) == CAP_TYPE_TIME);

        cap_time_set_free(&cap, cap_time_get_end(newcap));

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t depth = cap_time_get_depth(cap);

        uint64_t newbegin = cap_time_get_begin(newcap);
        uint64_t newend = cap_time_get_end(newcap);
        uint64_t newdepth = cap_time_get_depth(newcap);

        if (cap_node_update(cap, cn) && cap_node_insert(newcap, newcn, cn) && sched_update(newcn, hartid, newbegin, newend, depth, current->pid, newdepth))
                return S3K_OK;

        return S3K_ERROR;
}

static inline uint64_t time_revoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);

        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t free = cap_time_get_free(cap);
        uint64_t depth = cap_time_get_depth(cap);

        preemption_enable();
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
                /* Enable preemption */
                preemption_enable();
        }
        preemption_disable();

        cap_time_set_free(&cap, begin);

        if (sched_update(cn, hartid, begin, free, 255, current->pid, depth) && cap_node_update(cap, cn))
                return S3K_OK;
        return S3K_ERROR;
}

static inline uint64_t time_delete_cap(cap_node_t* cn, cap_t cap)
{
        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t depth = cap_time_get_depth(cap);
        if (sched_update(cn, hartid, begin, end, depth, current->pid, 255) && cap_node_delete(cn))
                return S3K_OK;
        return S3K_ERROR;
}

void syscall_handle_time(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_TIME);
        preemption_disable();
        switch (regs->a7) {
                case S3K_SYSNR_READ_CAP:
                        regs->a0 = S3K_OK;
                        regs->a1 = cap.word0;
                        regs->a2 = cap.word1;
                        break;

                case S3K_SYSNR_MOVE_CAP:
                        regs->a0 = cap_node_move(cn, proc_get_cap_node(current, regs->a1)) ? S3K_OK : S3K_ERROR;
                        break;

                case S3K_SYSNR_DELETE_CAP:
                        regs->a0 = time_delete_cap(cn, cap);
                        break;

                case S3K_SYSNR_REVOKE_CAP:
                        regs->a0 = time_revoke_cap(cn, cap);
                        break;

                case S3K_SYSNR_DERIVE_CAP: {
                        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
                        cap_t newcap = (cap_t){regs->a2, regs->a3};
                        regs->a0 = time_derive_cap(cn, cap, newcn, newcap);
                } break;

                default:
                        regs->a0 = S3K_UNIMPLEMENTED;
                        break;
        }
        regs->pc += 4;
        preemption_enable();
}
