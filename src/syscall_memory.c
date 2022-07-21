#include "syscall_memory.h"

#include "pmp.h"
#include "preemption.h"
#include "s3k_consts.h"

static inline uint64_t memory_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_MEMORY);

        if (!cap_node_is_deleted(newcn))
                return S3K_COLLISION;

        if (!cap_can_derive(cap, newcap))
                return S3K_ILLEGAL_DERIVATION;

        kassert(cap_get_type(newcap) == CAP_TYPE_MEMORY || cap_get_type(newcap) == CAP_TYPE_PMP);

        if (cap_get_type(newcap) == CAP_TYPE_MEMORY)
                cap_memory_set_free(&cap, cap_memory_get_end(newcap));

        if (cap_get_type(newcap) == CAP_TYPE_PMP)
                cap_memory_set_free(&cap, pmp_napot_begin(cap_pmp_get_addr(newcap)));

        if (cap_node_update(cap, cn) && cap_node_insert(newcap, newcn, cn))
                return S3K_OK;
        return S3K_ERROR;
}

static inline uint64_t memory_revoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_MEMORY);

        preemption_enable();
        cap_node_revoke(cn);
        preemption_disable();

        cap_memory_set_free(&cap, cap_memory_get_begin(cap));

        if (cap_node_update(cap, cn))
                return S3K_OK;
        return S3K_ERROR;
}

void syscall_handle_memory(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_MEMORY);
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
                        regs->a0 = cap_node_delete(cn) ? S3K_OK : S3K_ERROR;
                        break;

                case S3K_SYSNR_REVOKE_CAP:
                        regs->a0 = memory_revoke_cap(cn, cap);
                        break;

                default:
                        regs->a0 = S3K_UNIMPLEMENTED;
                        break;
        }
        regs->pc += 4;
        preemption_enable();
}

void syscall_handle_pmp(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_PMP);
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
                        regs->a0 = cap_node_delete(cn) ? S3K_OK : S3K_ERROR;

                default:
                        regs->a0 = S3K_UNIMPLEMENTED;
                        break;
        }
        regs->pc += 4;
        preemption_enable();
}
