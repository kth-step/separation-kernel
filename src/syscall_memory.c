#include "syscall_memory.h"

#include "pmp.h"
#include "preemption.h"
#include "s3k_consts.h"

void syscall_memory_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_MEMORY);

        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
        cap_t newcap = (cap_t){regs->a2, regs->a3};

        if (!cap_node_is_deleted(newcn)) {
                preemption_disable();
                regs->a0 = S3K_COLLISION;
                regs->pc += 4;
                preemption_disable();
                return;
        }

        if (!cap_can_derive(cap, newcap)) {
                preemption_disable();
                regs->a0 = S3K_ILLEGAL_DERIVATION;
                regs->pc += 4;
                preemption_disable();
                return;
        }

        kassert(cap_get_type(newcap) == CAP_TYPE_MEMORY || cap_get_type(newcap) == CAP_TYPE_PMP);

        if (cap_get_type(newcap) == CAP_TYPE_MEMORY)
                cap_memory_set_free(&cap, cap_memory_get_end(newcap));

        if (cap_get_type(newcap) == CAP_TYPE_PMP)
                cap_memory_set_pmp(&cap, 1);

        preemption_disable();
        if (cap_node_update(cap, cn) && cap_node_insert(newcap, newcn, cn)) {
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_ERROR;
        }
        regs->pc += 4;
        preemption_enable();
}

void syscall_memory_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_MEMORY);

        cap_node_revoke(cn);

        cap_memory_set_free(&cap, cap_memory_get_begin(cap));
        cap_memory_set_pmp(&cap, 0);

        preemption_disable();
        regs->a0 = cap_node_update(cap, cn) ? S3K_OK : S3K_ERROR;
        regs->pc += 4;
        preemption_enable();
}
