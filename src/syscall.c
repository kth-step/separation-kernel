// See LICENSE file for copyright and license details.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cap_node.h"
#include "preemption.h"
#include "proc.h"
#include "s3k_consts.h"
#include "syscall_ipc.h"
#include "syscall_supervisor.h"
#include "syscall_time.h"
#include "trap.h"
#include "utils.h"

/* Function declarations */
static void syscall_get_pid(void) __attribute__((noreturn));
static void syscall_read_reg(void) __attribute__((noreturn));
static void syscall_write_reg(void) __attribute__((noreturn));
static void syscall_yield(void) __attribute__((noreturn));
static void syscall_unimplemented(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_read_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_move_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_delete_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_revoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_memory_derive_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_channels_derive_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_receiver_derive_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_server_derive_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
static void syscall_supervisor_derive_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));

typedef void (*cap_handler_t)(cap_node_t*, cap_t) __attribute__((noreturn));
typedef void (*handler_t)(void) __attribute__((noreturn));

static const handler_t handlers[NUM_OF_SYSNR - S3K_SYSNR_GET_PID] = {
    syscall_get_pid, syscall_read_reg, syscall_write_reg, syscall_yield};

static const cap_handler_t cap_handlers[NUM_OF_CAP_TYPES][8] = {
    /* EMPTY */
    [CAP_TYPE_EMPTY] =
        {
            syscall_read_cap,
            syscall_unimplemented,
            syscall_unimplemented,
            syscall_unimplemented,
            syscall_unimplemented,
            syscall_unimplemented,
        },
    /* MEMORY */
    [CAP_TYPE_MEMORY] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_revoke_cap,
            syscall_memory_derive_cap,
            syscall_unimplemented,
        },
    [CAP_TYPE_PMP] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_unimplemented,
            syscall_unimplemented,
            syscall_unimplemented,
        },
    /* TIME */
    [CAP_TYPE_TIME] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_time_delete_cap,
            syscall_time_revoke_cap,
            syscall_time_derive_cap,
            syscall_unimplemented,
        },
    /* IPC */
    [CAP_TYPE_CHANNELS] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_revoke_cap,
            syscall_channels_derive_cap,
            syscall_unimplemented,
        },
    [CAP_TYPE_RECEIVER] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_revoke_cap,
            syscall_receiver_derive_cap,
            syscall_receiver_invoke_cap,
        },
    [CAP_TYPE_SENDER] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_unimplemented,
            syscall_unimplemented,
            syscall_sender_invoke_cap,
        },
    [CAP_TYPE_SERVER] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_revoke_cap,
            syscall_server_derive_cap,
            syscall_server_invoke_cap,
        },
    [CAP_TYPE_CLIENT] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_unimplemented,
            syscall_unimplemented,
            syscall_client_invoke_cap,
        },
    /* SUPERVISOR */
    [CAP_TYPE_SUPERVISOR] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_revoke_cap,
            syscall_supervisor_derive_cap,
            syscall_supervisor_invoke_cap,
        },
};

void syscall_handler(void)
{
        uint64_t syscall_nr = current->regs.a7;
        if (syscall_nr < S3K_SYSNR_GET_PID) {
                cap_node_t* cn = proc_get_cap_node(current, current->regs.a0);
                cap_t cap = cap_node_get_cap(cn);
                cap_handlers[cap_get_type(cap)][syscall_nr](cn, cap);
        } else if (syscall_nr < NUM_OF_SYSNR) {
                handlers[syscall_nr - S3K_SYSNR_GET_PID]();
        } else {
                exception_handler(8, 0);
        }
}

void syscall_unimplemented(cap_node_t* cn, cap_t cap)
{
        trap_syscall_exit(S3K_UNIMPLEMENTED);
}

void syscall_read_cap(cap_node_t* cn, cap_t cap)
{
        preemption_disable();
        current->regs.a0 = cap.word0;
        current->regs.a1 = cap.word1;
        trap_syscall_exit3();
}

void syscall_move_cap(cap_node_t* cn, cap_t cap)
{
        cap_node_t* cndest = proc_get_cap_node(current, current->regs.a1);
        if (!cap_node_is_deleted(cndest)) {
                trap_syscall_exit(S3K_COLLISION);
        } else {
                preemption_disable();
                cap_node_move(cn, cndest);
                trap_syscall_exit(S3K_OK);
        }
}

void syscall_memory_derive_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_MEMORY);
        cap_node_t* newcn = proc_get_cap_node(current, current->regs.a1);
        cap_t newcap = (cap_t){current->regs.a2, current->regs.a3};

        if (!cap_node_is_deleted(newcn))
                trap_syscall_exit(S3K_COLLISION);

        if (!cap_can_derive_memory(cap, newcap))
                trap_syscall_exit(S3K_ILLEGAL_DERIVATION);

        if (cap_get_type(newcap) == CAP_TYPE_MEMORY)
                cap_memory_set_free(&cap, cap_memory_get_end(newcap));
        if (cap_get_type(newcap) == CAP_TYPE_PMP)
                cap_memory_set_pmp(&cap, 1);

        preemption_disable();
        cap_node_update(cap, cn);
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_channels_derive_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);
        cap_node_t* newcn = proc_get_cap_node(current, current->regs.a1);
        cap_t newcap = (cap_t){current->regs.a2, current->regs.a3};

        if (!cap_node_is_deleted(newcn))
                trap_syscall_exit(S3K_COLLISION);

        if (!cap_can_derive_channels(cap, newcap))
                trap_syscall_exit(S3K_ILLEGAL_DERIVATION);

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

void syscall_receiver_derive_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_RECEIVER);
        cap_node_t* newcn = proc_get_cap_node(current, current->regs.a1);
        cap_t newcap = (cap_t){current->regs.a2, current->regs.a3};

        if (!cap_node_is_deleted(newcn))
                trap_syscall_exit(S3K_COLLISION);

        if (!cap_can_derive_receiver(cap, newcap))
                trap_syscall_exit(S3K_ILLEGAL_DERIVATION);

        preemption_disable();
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_server_derive_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SERVER);
        cap_node_t* newcn = proc_get_cap_node(current, current->regs.a1);
        cap_t newcap = (cap_t){current->regs.a2, current->regs.a3};

        if (!cap_node_is_deleted(newcn))
                trap_syscall_exit(S3K_COLLISION);

        if (!cap_can_derive_server(cap, newcap))
                trap_syscall_exit(S3K_ILLEGAL_DERIVATION);

        preemption_disable();
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_supervisor_derive_cap(cap_node_t* cn, cap_t cap)
{
        cap_node_t* newcn = proc_get_cap_node(current, current->regs.a1);
        cap_t newcap = (cap_t){current->regs.a2, current->regs.a3};

        if (!cap_node_is_deleted(newcn))
                trap_syscall_exit(S3K_COLLISION);

        if (!cap_can_derive_supervisor(cap, newcap))
                trap_syscall_exit(S3K_ILLEGAL_DERIVATION);

        if (cap_get_type(newcap) == CAP_TYPE_SUPERVISOR)
                cap_supervisor_set_free(&cap, cap_supervisor_get_end(newcap));

        preemption_disable();
        cap_node_update(cap, cn);
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_revoke_cap(cap_node_t* cn, cap_t cap)
{
        switch (cap_get_type(cap)) {
        case CAP_TYPE_MEMORY:
                cap_node_revoke(cn, cap, cap_is_child_memory);
                cap_memory_set_free(&cap, cap_memory_get_begin(cap));
                cap_memory_set_pmp(&cap, 0);
                break;
        case CAP_TYPE_SUPERVISOR:
                cap_node_revoke(cn, cap, cap_is_child_supervisor);
                cap_supervisor_set_free(&cap, cap_supervisor_get_begin(cap));
                break;
        case CAP_TYPE_CHANNELS:
                cap_node_revoke(cn, cap, cap_is_child_channels);
                cap_channels_set_free(&cap, cap_channels_get_begin(cap));
                break;
        case CAP_TYPE_RECEIVER:
                cap_node_revoke(cn, cap, cap_is_child_receiver);
                break;
        case CAP_TYPE_SERVER:
                cap_node_revoke(cn, cap, cap_is_child_server);
                break;
        default:
                kassert(0);
                break;
        }

        preemption_disable();
        cap_node_update(cap, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_delete_cap(cap_node_t* cn, cap_t cap)
{
        preemption_disable();
        cap_node_delete(cn);
        trap_syscall_exit2(S3K_OK);
}

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
        trap_yield();
}
