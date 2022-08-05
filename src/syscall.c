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

void syscall_get_pid(void)
{
        trap_syscall_exit(current->pid);
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

void syscall_read_cap(void)
{
        cap_node_t* cap_node = proc_get_cap_node(current, current->regs.a0);
        if (!cap_node)
                trap_syscall_exit(S3K_ERROR);
        cap_t cap = cap_node_get_cap(cap_node);
        preemption_disable();
        current->regs.a1 = cap.word0;
        current->regs.a2 = cap.word1;
        trap_syscall_exit2(S3K_OK);
}

void syscall_move_cap(void)
{
        cap_node_t* cnsrc = proc_get_cap_node(current, current->regs.a0);
        cap_node_t* cndest = proc_get_cap_node(current, current->regs.a1);
        if (!cnsrc || !cndest)
                trap_syscall_exit(S3K_ERROR);
        if (cap_node_is_deleted(cnsrc))
                trap_syscall_exit(S3K_EMPTY);
        if (!cap_node_is_deleted(cndest))
                trap_syscall_exit(S3K_COLLISION);

        preemption_disable();
        cap_node_move(cnsrc, cndest);
        trap_syscall_exit(S3K_OK);
}

void syscall_delete_cap(void)
{
        cap_node_t* cn = proc_get_cap_node(current, current->regs.a0);
        if (!cn)
                trap_syscall_exit(S3K_ERROR);
        preemption_disable();
        cap_node_delete(cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_derive_cap(void)
{
        cap_node_t* cn = proc_get_cap_node(current, current->regs.a0);
        cap_node_t* newcn = proc_get_cap_node(current, current->regs.a1);
        if (!cn || !newcn)
                trap_syscall_exit(S3K_ERROR);
        cap_t cap = cap_node_get_cap(cn);
        cap_t newcap = (cap_t){current->regs.a2, current->regs.a3};
        if (!newcn)
                trap_syscall_exit(S3K_ERROR);
        if (!cap_node_is_deleted(newcn))
                trap_syscall_exit(S3K_COLLISION);

        switch (cap_get_type(cap)) {
        case CAP_TYPE_MEMORY:
                syscall_memory_derive_cap(cn, cap, newcn, newcap);
        case CAP_TYPE_TIME:
                syscall_time_derive_cap(cn, cap, newcn, newcap);
        case CAP_TYPE_CHANNELS:
                syscall_channels_derive_cap(cn, cap, newcn, newcap);
        case CAP_TYPE_RECEIVER:
                syscall_receiver_derive_cap(cn, cap, newcn, newcap);
        case CAP_TYPE_SERVER:
                syscall_server_derive_cap(cn, cap, newcn, newcap);
        case CAP_TYPE_SUPERVISOR:
                syscall_supervisor_derive_cap(cn, cap, newcn, newcap);
        default:
                trap_syscall_exit(S3K_UNIMPLEMENTED);
        }
}

void syscall_revoke_cap(void)
{
        cap_node_t* cn = proc_get_cap_node(current, current->regs.a0);
        if (!cn)
                trap_syscall_exit(S3K_ERROR);
        cap_t cap = cap_node_get_cap(cn);
        switch (cap_get_type(cap)) {
        case CAP_TYPE_MEMORY:
                syscall_memory_revoke_cap(cn, cap);
        case CAP_TYPE_TIME:
                syscall_time_revoke_cap(cn, cap);
        case CAP_TYPE_CHANNELS:
                syscall_channels_revoke_cap(cn, cap);
        case CAP_TYPE_RECEIVER:
                syscall_receiver_revoke_cap(cn, cap);
        case CAP_TYPE_SERVER:
                syscall_server_revoke_cap(cn, cap);
        case CAP_TYPE_SUPERVISOR:
                syscall_supervisor_revoke_cap(cn, cap);
        default:
                trap_syscall_exit(S3K_UNIMPLEMENTED);
        }
}

void syscall_invoke_cap(void)
{
        cap_node_t* cn = proc_get_cap_node(current, current->regs.a0);
        if (!cn)
                trap_syscall_exit(S3K_ERROR);
        cap_t cap = cap_node_get_cap(cn);
        switch (cap_get_type(cap)) {
        case CAP_TYPE_RECEIVER:
                syscall_receiver_invoke_cap(cn, cap);
        case CAP_TYPE_SENDER:
                syscall_sender_invoke_cap(cn, cap);
        case CAP_TYPE_SERVER:
                syscall_server_invoke_cap(cn, cap);
        case CAP_TYPE_CLIENT:
                syscall_client_invoke_cap(cn, cap);
        case CAP_TYPE_SUPERVISOR:
                syscall_supervisor_invoke_cap(cn, cap);
        default:
                trap_syscall_exit(S3K_UNIMPLEMENTED);
        }
}
