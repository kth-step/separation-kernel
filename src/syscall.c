// See LICENSE file for copyright and license details.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cap_node.h"
#include "preemption.h"
#include "proc.h"
#include "s3k_consts.h"
#include "syscall_ipc.h"
#include "syscall_memory.h"
#include "syscall_supervisor.h"
#include "syscall_time.h"
#include "trap.h"
#include "utils.h"

typedef void (*handler_t)(cap_node_t*, cap_t) __attribute__((noreturn));
typedef void (*handler_derive_t)(cap_node_t*, cap_t, cap_node_t*, cap_t) __attribute__((noreturn));

static const handler_t revoke_handlers[NUM_OF_CAP_TYPES] = {
    [CAP_TYPE_MEMORY] = syscall_memory_revoke_cap,
    [CAP_TYPE_PMP] = (handler_t)syscall_unimplemented,
    [CAP_TYPE_TIME] = syscall_time_revoke_cap,
    [CAP_TYPE_CHANNELS] = syscall_channels_revoke_cap,
    [CAP_TYPE_RECEIVER] = syscall_receiver_revoke_cap,
    [CAP_TYPE_SENDER] = (handler_t)syscall_unimplemented,
    [CAP_TYPE_SERVER] = syscall_server_revoke_cap,
    [CAP_TYPE_CLIENT] = (handler_t)syscall_unimplemented,
    [CAP_TYPE_SUPERVISOR] = syscall_supervisor_revoke_cap};

static const handler_t invoke_handlers[NUM_OF_CAP_TYPES] = {
    [CAP_TYPE_MEMORY] = (handler_t)syscall_unimplemented,
    [CAP_TYPE_PMP] = (handler_t)syscall_unimplemented,
    [CAP_TYPE_TIME] = (handler_t)syscall_unimplemented,
    [CAP_TYPE_CHANNELS] = (handler_t)syscall_unimplemented,
    [CAP_TYPE_RECEIVER] = syscall_receiver_invoke_cap,
    [CAP_TYPE_SENDER] = syscall_sender_invoke_cap,
    [CAP_TYPE_SERVER] = syscall_server_invoke_cap,
    [CAP_TYPE_CLIENT] = syscall_client_invoke_cap,
    [CAP_TYPE_SUPERVISOR] = syscall_supervisor_invoke_cap};

static const handler_derive_t derive_handlers[NUM_OF_CAP_TYPES] = {
    [CAP_TYPE_MEMORY] = syscall_memory_derive_cap,
    [CAP_TYPE_PMP] = (handler_derive_t)syscall_unimplemented,
    [CAP_TYPE_TIME] = syscall_time_derive_cap,
    [CAP_TYPE_CHANNELS] = syscall_channels_derive_cap,
    [CAP_TYPE_RECEIVER] = syscall_receiver_derive_cap,
    [CAP_TYPE_SENDER] = (handler_derive_t)syscall_unimplemented,
    [CAP_TYPE_SERVER] = syscall_server_derive_cap,
    [CAP_TYPE_CLIENT] = (handler_derive_t)syscall_unimplemented,
    [CAP_TYPE_SUPERVISOR] = syscall_supervisor_derive_cap};

void syscall_unimplemented(void)
{
        trap_syscall_exit1(S3K_UNIMPLEMENTED);
}

void syscall_read_cap(cap_node_t* cap_node, cap_t cap)
{
        preemption_disable();
        current->regs.a1 = cap.word0;
        current->regs.a2 = cap.word1;
        trap_syscall_exit2(S3K_OK);
}

void syscall_move_cap(cap_node_t* cap_node, cap_t cap)
{
        cap_node_t* cndest = proc_get_cap_node(current, current->regs.a1);
        if (!cap_node_is_deleted(cndest))
                trap_syscall_exit1(S3K_COLLISION);
        preemption_disable();
        cap_node_move(cap_node, cndest);
        trap_syscall_exit2(S3K_OK);
}

void syscall_delete_cap(cap_node_t* cap_node, cap_t cap)
{
        preemption_disable();
        cap_node_delete(cap_node);
        trap_syscall_exit2(S3K_OK);
}

void syscall_derive_cap(cap_node_t* cap_node, cap_t cap)
{
        cap_node_t* newcn = proc_get_cap_node(current, current->regs.a1);
        if (!cap_node_is_deleted(newcn))
                trap_syscall_exit1(S3K_COLLISION);
        cap_t newcap = (cap_t){current->regs.a2, current->regs.a3};
        derive_handlers[cap_get_type(cap)](cap_node, cap, newcn, newcap);
}

void syscall_revoke_cap(cap_node_t* cap_node, cap_t cap)
{
        revoke_handlers[cap_get_type(cap)](cap_node, cap);
}

void syscall_invoke_cap(cap_node_t* cap_node, cap_t cap)
{
        invoke_handlers[cap_get_type(cap)](cap_node, cap);
}

void syscall_get_pid(void)
{
        trap_syscall_exit1(current->pid);
}

void syscall_read_reg(void)
{
        preemption_disable();
        trap_syscall_exit2(proc_read_register(current, current->regs.a0));
}

void syscall_write_reg(void)
{
        preemption_disable();
        trap_syscall_exit2(proc_write_register(current, current->regs.a0, current->regs.a1));
}

void syscall_yield(void)
{
        preemption_disable();
        current->regs.timeout = read_timeout(read_csr(mhartid));
        current->regs.pc += 4;
        sched_yield();
}
