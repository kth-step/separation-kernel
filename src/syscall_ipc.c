#include "syscall_ipc.h"

#include "interprocess_move.h"
#include "preemption.h"
#include "s3k_consts.h"
#include "trap.h"

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

static inline void pass_message(proc_t* sender, proc_t* receiver, uint64_t src, uint64_t dest, uint64_t can_grant)
{
        if (src < N_CAPS && dest < N_CAPS && can_grant) {
                interprocess_move(current, receiver, src, dest);
        }
        receiver->regs.a1 = sender->regs.a1;
        receiver->regs.a2 = sender->regs.a2;
        receiver->regs.a3 = sender->regs.a3;
        receiver->regs.a4 = sender->regs.a4;
}

void syscall_receiver_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_RECEIVER);
        kassert(regs = &current->regs);

        uint64_t channel = cap_receiver_get_channel(cap);
        if (!set_listener(cn, current, channel)) {
                preemption_disable();
                regs->a0 = S3K_EMPTY;
                regs->pc += 4;
        } else {
                preemption_disable();
                if (proc_receiver_wait(current, channel)) {
                        /* Process wait for message */
                        trap_yield();
                }
        }
}

void syscall_sender_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SENDER);
        kassert(regs = &current->regs);

        uint64_t channel = cap_sender_get_channel(cap);
        proc_t* receiver = get_listener(channel);

        preemption_disable();
        if (receiver == NULL || !proc_sender_acquire(receiver, channel)) {
                regs->a0 = S3K_NO_RECEIVER;
                regs->pc += 4;
        } else {
                uint64_t src = regs->a5 & 0xFF;
                uint64_t dest = receiver->regs.a5 & 0xFF;
                uint64_t can_grant = cap_sender_get_grant(cap);
                pass_message(current, receiver, src, dest, can_grant);
                receiver->regs.a0 = S3K_OK;
                receiver->regs.pc += 4;
                proc_sender_release(receiver);

                regs->a0 = S3K_OK;
                regs->pc += 4;
                if (regs->a6)
                        trap_yield();
        }
}

void syscall_server_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SERVER);
        kassert(regs == &current->regs);
}

void syscall_client_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SERVER);
        kassert(regs == &current->regs);
}
