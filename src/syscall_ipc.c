// See LICENSE file for copyright and license details.
#include "syscall_ipc.h"

#include "interprocess_move.h"
#include "preemption.h"
#include "s3k_consts.h"
#include "sched.h"

proc_t* listeners[N_CHANNELS];

static inline bool set_listener(cap_node_t* cn, proc_t* proc, uint64_t channel)
{
        while (1) {
                proc_t* old = listeners[channel];
                if (cap_node_is_deleted(cn))
                        return false;
                if (compare_and_set(&listeners[channel], old, proc))
                        return true;
        }
}

static inline proc_t* get_listener(uint64_t channel)
{
        return listeners[channel];
}

static inline void pass_message(proc_t* sender, proc_t* receiver, uint64_t src, uint64_t dest,
                                uint64_t can_grant)
{
        if (src < N_CAPS && dest < N_CAPS && can_grant) {
                interprocess_move(current, receiver, src, dest);
        }
        receiver->regs.a1 = sender->regs.a1;
        receiver->regs.a2 = sender->regs.a2;
        receiver->regs.a3 = sender->regs.a3;
        receiver->regs.a4 = sender->regs.a4;
}

void syscall_channels_revoke_cap(cap_node_t* cn, cap_t cap)
{
        cap_node_revoke(cn, cap, cap_is_child_channels);
        cap_channels_set_free(&cap, cap_channels_get_begin(cap));

        preemption_disable();
        cap_node_update(cap, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_receiver_revoke_cap(cap_node_t* cn, cap_t cap)
{
        cap_node_revoke(cn, cap, cap_is_child_receiver);
        trap_syscall_exit1(S3K_OK);
}

void syscall_server_revoke_cap(cap_node_t* cn, cap_t cap)
{
        cap_node_revoke(cn, cap, cap_is_child_server);
        trap_syscall_exit1(S3K_OK);
}

void syscall_channels_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);

        if (!cap_can_derive_channels(cap, newcap))
                trap_syscall_exit1(S3K_ILLEGAL_DERIVATION);

        if (cap_get_type(newcap) == CAP_TYPE_CHANNELS)
                cap_channels_set_free(&cap, cap_channels_get_end(newcap));

        if (cap_get_type(newcap) == CAP_TYPE_RECEIVER)
                cap_channels_set_free(&cap, cap_receiver_get_channel(newcap) + 1);

        if (cap_get_type(newcap) == CAP_TYPE_SERVER)
                cap_channels_set_free(&cap, cap_server_get_channel(newcap) + 1);

        preemption_disable();
        cap_node_update(cap, cn);
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_receiver_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_RECEIVER);

        if (!cap_can_derive_receiver(cap, newcap))
                trap_syscall_exit1(S3K_ILLEGAL_DERIVATION);

        preemption_disable();
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_server_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SERVER);

        if (!cap_can_derive_server(cap, newcap))
                trap_syscall_exit1(S3K_ILLEGAL_DERIVATION);

        preemption_disable();
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_receiver_invoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_RECEIVER);

        uint64_t channel = cap_receiver_get_channel(cap);
        if (!set_listener(cn, current, channel))
                trap_syscall_exit1(S3K_EMPTY);
        preemption_disable();
        if (proc_receiver_wait(current, channel)) {
                /* Process wait for message */
                sched_yield();
        }
        trap_syscall_exit1(S3K_ERROR);
}

void syscall_sender_invoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SENDER);

        uint64_t channel = cap_sender_get_channel(cap);
        proc_t* receiver = get_listener(channel);

        preemption_disable();
        if (receiver == NULL || !proc_sender_acquire(receiver, channel))
                trap_syscall_exit1(S3K_NO_RECEIVER);

        uint64_t src = current->regs.a5 & 0xFF;
        uint64_t dest = receiver->regs.a5 & 0xFF;
        uint64_t can_grant = cap_sender_get_grant(cap);
        pass_message(current, receiver, src, dest, can_grant);
        receiver->regs.a0 = S3K_OK;
        receiver->regs.pc += 4;
        proc_sender_release(receiver);

        if (current->regs.a6)
                sched_yield();
        trap_syscall_exit1(S3K_OK);
}

void syscall_server_invoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SERVER);
        trap_syscall_exit1(S3K_UNIMPLEMENTED);
}

void syscall_client_invoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_CLIENT);
        trap_syscall_exit1(S3K_UNIMPLEMENTED);
}
