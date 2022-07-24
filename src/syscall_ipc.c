#include "syscall_ipc.h"

#include "ipc.h"
#include "preemption.h"
#include "s3k_consts.h"
#include "trap.h"

void syscall_channels_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);

        cap_node_revoke(cn);

        cap_channels_set_free(&cap, cap_channels_get_begin(cap));

        preemption_disable();
        regs->a0 = cap_node_update(cap, cn) ? S3K_OK : S3K_ERROR;
        regs->pc += 4;
        preemption_enable();
}

void syscall_channels_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);

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

        kassert(cap_get_type(newcap) == CAP_TYPE_CHANNELS || cap_get_type(newcap) == CAP_TYPE_RECEIVER);

        if (cap_get_type(newcap) == CAP_TYPE_CHANNELS)
                cap_channels_set_free(&cap, cap_channels_get_end(newcap));

        if (cap_get_type(newcap) == CAP_TYPE_RECEIVER)
                cap_channels_set_free(&cap, cap_receiver_get_channel(newcap) + 1);

        preemption_disable();
        if (cap_node_update(cap, cn) && cap_node_insert(newcap, newcn, cn))
                regs->a0 = S3K_OK;
        else
                regs->a0 = S3K_ERROR;
        regs->pc += 4;
        preemption_enable();
}

void syscall_receiver_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);

        cap_node_revoke(cn);

        preemption_disable();
        regs->a0 = S3K_OK;
        regs->pc += 4;
        preemption_enable();
}

void syscall_receiver_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_RECEIVER);

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

        preemption_disable();
        regs->a0 = cap_node_insert(newcap, newcn, cn) ? S3K_OK : S3K_ERROR;
        regs->pc += 4;
        preemption_enable();
}

void syscall_receiver_receive(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_RECEIVER);

        preemption_disable();
        uint64_t channel = cap_receiver_get_channel(cap);
        if (ipc_subscribe(current, channel)) {
                if (proc_receiver_wait(current, channel)) {
                        trap_yield();
                }
                ipc_unsubscribe(channel);
                trap_yield();
        }
        regs->a0 = S3K_ERROR;
        regs->a1 = 0;
        regs->a2 = 0;
        regs->a3 = 0;
        regs->a4 = 0;
        regs->pc += 4;
        preemption_enable();
}

void syscall_sender_send(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SENDER);

        uint64_t channel = cap_sender_get_channel(cap);
        proc_t* receiver = ipc_get_subscriber(channel);
        preemption_disable();
        if (receiver == NULL || !proc_sender_acquire(receiver, channel)) {
                regs->a0 = S3K_NO_RECEIVER;
                regs->pc += 4;
        } else {
                regs->a0 = S3K_OK;
                regs->pc += 4;

                receiver->regs.a0 = S3K_OK;
                receiver->regs.pc += 4;
                receiver->regs.a1 = regs->a1;
                receiver->regs.a2 = regs->a2;
                receiver->regs.a3 = regs->a3;
                receiver->regs.a4 = regs->a4;
                ipc_unsubscribe(channel);
                proc_sender_release(receiver);
        }
        preemption_enable();
}
