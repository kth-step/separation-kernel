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
static void syscall_get_pid(registers_t* regs);
static void syscall_read_reg(registers_t* regs);
static void syscall_write_reg(registers_t* regs);
static void syscall_yield(registers_t* regs);
static void syscall_unimplemented(registers_t* regs, cap_node_t* cn, cap_t cap);
static void syscall_read_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
static void syscall_move_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
static void syscall_delete_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
static void syscall_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
static void syscall_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap);

typedef void (*cap_handler_t)(registers_t*, cap_node_t*, cap_t);
typedef void (*handler_t)(registers_t*);

static const handler_t handlers[NUM_OF_SYSNR - S3K_SYSNR_GET_PID] = {syscall_get_pid, syscall_read_reg,
                                                                     syscall_write_reg, syscall_yield};

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
            syscall_derive_cap,
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
            syscall_derive_cap,
            syscall_unimplemented,
        },
    [CAP_TYPE_RECEIVER] =
        {
            syscall_read_cap,
            syscall_move_cap,
            syscall_delete_cap,
            syscall_revoke_cap,
            syscall_derive_cap,
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
            syscall_derive_cap,
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
            syscall_derive_cap,
            syscall_supervisor_invoke_cap,
        },
};

void syscall_handler(registers_t* regs)
{
        kassert(regs == &current->regs);
        uint64_t syscall_nr = regs->a7;
        if (syscall_nr < S3K_SYSNR_GET_PID) {
                cap_node_t* cn = proc_get_cap_node(current, regs->a0);
                cap_t cap = cap_node_get_cap(cn);
                cap_handlers[cap_get_type(cap)][syscall_nr](regs, cn, cap);
        } else if (syscall_nr < NUM_OF_SYSNR) {
                handlers[syscall_nr - S3K_SYSNR_GET_PID](regs);
        } else {
                exception_handler(regs, 8, 0);
        }
}

void syscall_unimplemented(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(regs == &current->regs);
        preemption_disable();
        regs->a0 = S3K_UNIMPLEMENTED;
        regs->pc += 4;
}

void syscall_read_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(regs == &current->regs);
        preemption_disable();
        regs->a0 = cap.word0;
        regs->a1 = cap.word1;
        regs->pc += 4;
}

void syscall_move_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(regs == &current->regs);
        cap_node_t* cndest = proc_get_cap_node(current, regs->a1);
        preemption_disable();
        if (!cap_node_is_deleted(cndest)) {
                regs->a0 = S3K_COLLISION;
                regs->pc += 4;
        } else {
                cap_node_move(cn, cndest);
                regs->a0 = S3K_OK;
                regs->pc += 4;
        }
}

void syscall_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(regs == &current->regs);
        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
        cap_t newcap = (cap_t){regs->a2, regs->a3};
        cap_t updcap = cap;

        switch (cap_get_type(newcap)) {
        case CAP_TYPE_MEMORY:
                cap_memory_set_free(&updcap, cap_memory_get_end(newcap));
                break;
        case CAP_TYPE_PMP:
                cap_memory_set_pmp(&updcap, 1);
                break;
        case CAP_TYPE_SUPERVISOR:
                cap_supervisor_set_free(&updcap, cap_supervisor_get_end(newcap));
                break;
        case CAP_TYPE_CHANNELS:
                cap_channels_set_free(&updcap, cap_channels_get_end(newcap));
                break;
        case CAP_TYPE_RECEIVER:
                cap_channels_set_free(&updcap, cap_receiver_get_channel(newcap) + 1);
                break;
        case CAP_TYPE_SERVER:
                cap_channels_set_free(&updcap, cap_server_get_channel(newcap) + 1);
                break;
        case CAP_TYPE_SENDER:
                break;
        case CAP_TYPE_CLIENT:
                break;
        default:
                kassert(0);
                break;
        }

        if (!cap_node_is_deleted(newcn)) {
                preemption_disable();
                regs->a0 = S3K_COLLISION;
                regs->pc += 4;
        } else if (!cap_can_derive(cap, newcap)) {
                preemption_disable();
                regs->a0 = S3K_ILLEGAL_DERIVATION;
                regs->pc += 4;
        } else {
                preemption_disable();
                cap_node_update(updcap, cn);
                cap_node_insert(newcap, newcn, cn);
                regs->a0 = S3K_OK;
                regs->pc += 4;
        }
}

void syscall_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(regs == &current->regs);
        cap_node_revoke(cn, cap);

        switch (cap_get_type(cap)) {
        case CAP_TYPE_MEMORY:
                cap_memory_set_free(&cap, cap_memory_get_begin(cap));
                cap_memory_set_pmp(&cap, 0);
                break;
        case CAP_TYPE_SUPERVISOR:
                cap_supervisor_set_free(&cap, cap_supervisor_get_begin(cap));
                break;
        case CAP_TYPE_CHANNELS:
                cap_channels_set_free(&cap, cap_channels_get_begin(cap));
                break;
        case CAP_TYPE_RECEIVER:
                break;
        case CAP_TYPE_SERVER:
                break;
        default:
                kassert(0);
                break;
        }

        preemption_disable();
        cap_node_update(cap, cn);
        regs->a0 = S3K_OK;
        regs->pc += 4;
}

void syscall_delete_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(regs == &current->regs);
        preemption_disable();
        cap_node_delete(cn);
        regs->a0 = S3K_OK;
        regs->pc += 4;
}

void syscall_get_pid(registers_t* regs)
{
        kassert(regs == &current->regs);
        preemption_disable();
        /* Get the process ID */
        regs->a0 = current->pid;
        regs->pc += 4;
}

void syscall_read_reg(registers_t* regs)
{
        kassert(regs == &current->regs);
        preemption_disable();
        regs->a0 = proc_read_register(current, regs->a0);
        regs->pc += 4;
}

void syscall_write_reg(registers_t* regs)
{
        kassert(regs == &current->regs);
        preemption_disable();
        regs->a0 = proc_write_register(current, regs->a0, regs->a1);
        regs->pc += 4;
}

void syscall_yield(registers_t* regs)
{
        kassert(regs == &current->regs);
        preemption_disable();
        regs->timeout = read_timeout(read_csr(mhartid));
        regs->pc += 4;
        trap_yield();
}
