// See LICENSE file for copyright and license details.

#include "syscall.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cap_node.h"
#include "consts.h"
#include "kprint.h"
#include "lock.h"
#include "preemption.h"
#include "proc.h"
#include "proc_state.h"
#include "sched.h"
#include "trap.h"

/*** INTERNAL FUNCTION DECLARATIONS ***/
/* For moving capability between processes */
static uint64_t interprocess_move(proc_t* src_proc, uint64_t src_cidx, proc_t* dest_proc, uint64_t dest_cidx);
/* Hook used when capability is created, updated or moved. */
static void cap_update_hook(proc_t* proc, cap_node_t* node, cap_t cap);
/* Returns update capability for after revoke */
static cap_t revoke_update_cap(cap_t cap);
/* Returns update capability for after deriving new_cap */
static cap_t derive_update_cap(cap_t src_cap, cap_t new_cap);

static uint64_t syscall_invoke_supervisor(cap_t cap, uint64_t pid, uint64_t op, uint64_t arg0, uint64_t arg1);
static uint64_t syscall_invoke_receiver(cap_t cap, uint64_t msg0, uint64_t msg1, uint64_t msg2, uint64_t msg3);
static uint64_t syscall_invoke_sender(cap_t cap, uint64_t msg0, uint64_t msg1, uint64_t msg2, uint64_t msg3,
                                      uint64_t src_cidx);
static uint64_t syscall_invoke_server(cap_t cap, uint64_t msg0, uint64_t msg1, uint64_t msg2, uint64_t msg3,
                                      uint64_t src_cidx, uint64_t flags);
static uint64_t syscall_invoke_client(cap_t cap, uint64_t msg0, uint64_t msg1, uint64_t msg2, uint64_t msg3,
                                      uint64_t src_cidx);

static lock_t receivers_lock;
static proc_t* receivers[N_CHANNELS][2];

/*** SYSTEM CALLS ***/

uint64_t syscall_unimplemented(void)
{
    return ERROR_UNIMPLEMENTED;
}

uint64_t syscall_read_cap(uint64_t cidx)
{
    kassert(current != NULL);
    cap_node_t* node = proc_get_cap_node(current, cidx);
    cap_t cap = cap_node_get_cap(node);
    current->regs.a1 = cap.word0;
    current->regs.a2 = cap.word1;
    return ERROR_OK;
}

uint64_t syscall_move_cap(uint64_t src_cidx, uint64_t dest_cidx)
{
    kassert(current != NULL);
    cap_node_t* src_node = proc_get_cap_node(current, src_cidx);
    cap_t cap = cap_node_get_cap(src_node);
    cap_node_t* dest_node = proc_get_cap_node(current, dest_cidx);
    if (cap_node_is_deleted(src_node))
        return ERROR_EMPTY;
    if (!cap_node_is_deleted(dest_node))
        return ERROR_COLLISION;
    return cap_node_move(cap, src_node, dest_node) ? ERROR_OK : ERROR_EMPTY;
}

uint64_t syscall_delete_cap(uint64_t cidx)
{
    kassert(current != NULL);
    cap_node_t* node = proc_get_cap_node(current, cidx);
    cap_t cap = cap_node_get_cap(node);
    if (cap_node_is_deleted(node) || !cap_node_delete(node))
        return ERROR_EMPTY;
    cap_update_hook(NULL, node, cap);
    return ERROR_OK;
}

uint64_t syscall_revoke_cap(uint64_t cidx)
{
    kassert(current != NULL);
    /* Return ERROR_PREEMPTED if preemted */
    current->regs.a0 = ERROR_PREEMPTED;

    /* !!! ENABLE PREEMPTION !!! */
    preemption_enable();

    /* Get the current node and capability*/
    cap_node_t* node = proc_get_cap_node(current, cidx);
    cap_t cap = node->cap;

    if (!cap_node_is_deleted(node))
        return ERROR_EMPTY;

    while (!cap_node_is_deleted(node)) {
        cap_node_t* next_node = node->next;
        cap_t next_cap = next_node->cap;
        if (!cap_is_child(cap, next_cap))
            break;
        preemption_disable();
        if (cap_node_delete2(next_node, node))
            cap_update_hook(current, node, next_cap);
        preemption_enable();
    }

    preemption_disable();
    node->cap = revoke_update_cap(cap);
    cap_update_hook(current, node, cap);
    return ERROR_OK;
}

uint64_t syscall_derive_cap(uint64_t src_cidx, uint64_t dest_cidx, uint64_t word0, uint64_t word1)
{
    kassert(current != NULL);
    /* If we get preempted, return ERROR_PREEMPTED */
    current->regs.a0 = ERROR_PREEMPTED;

    /* !!! ENABLE PREEMPTION !!! */
    preemption_enable();

    cap_node_t* src_node = proc_get_cap_node(current, src_cidx);
    cap_t src_cap = cap_node_get_cap(src_node);
    cap_node_t* dest_node = proc_get_cap_node(current, dest_cidx);
    cap_t new_cap = (cap_t){word0, word1};

    /* Check if we can derive the capability */
    if (cap_node_is_deleted(src_node))
        return ERROR_EMPTY;
    if (!cap_node_is_deleted(dest_node))
        return ERROR_COLLISION;
    if (!cap_can_derive(src_cap, new_cap))
        return ERROR_ILLEGAL_DERIVATION;
    preemption_disable();
    src_node->cap = derive_update_cap(src_cap, new_cap);
    cap_update_hook(current, src_node, new_cap);
    return cap_node_insert(new_cap, dest_node, src_node) ? ERROR_OK : ERROR_EMPTY;
}

static uint64_t syscall_invoke_supervisor(cap_t cap, uint64_t pid, uint64_t op, uint64_t arg3, uint64_t arg4);

uint64_t syscall_invoke_cap(uint64_t cidx, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5,
                            uint64_t arg6, uint64_t arg7)
{
    cap_node_t* node = proc_get_cap_node(current, cidx);
    cap_t cap = cap_node_get_cap(node);
    switch (cap_get_type(cap)) {
    case CAP_TYPE_EMPTY:
        return ERROR_EMPTY;
    case CAP_TYPE_SUPERVISOR:
        /* arg1 -> pid */
        /* arg2 -> op */
        return syscall_invoke_supervisor(cap, arg1, arg2, arg3, arg4);
    case CAP_TYPE_RECEIVER:
        /* arg1 -> cap destination */
        /* arg2-5 -> message */
        return syscall_invoke_receiver(cap, arg1, arg2, arg3, arg4);
    case CAP_TYPE_SENDER:
        /* arg1-4 -> message */
        /* arg5 -> cap to send */
        return syscall_invoke_sender(cap, arg1, arg2, arg3, arg4, arg5);
    case CAP_TYPE_SERVER:
        /* arg1-4 -> message */
        /* arg5 -> cap to send */
        /* arg6 -> do receive */
        return syscall_invoke_server(cap, arg1, arg2, arg3, arg4, arg5, arg6);
    case CAP_TYPE_CLIENT:
        /* arg1-4 -> message */
        /* arg5 -> cap to send */
        return syscall_invoke_client(cap, arg1, arg2, arg3, arg4, arg5);
    default:
        return ERROR_UNIMPLEMENTED;
    }
}

uint64_t syscall_invoke_supervisor(cap_t cap, uint64_t pid, uint64_t op, uint64_t arg0, uint64_t arg1)
{
    kassert(cap_is_type(cap, CAP_TYPE_SUPERVISOR));

    /* We are only allowed to work with processor i if cap.free <= i < cap.end */
    if (cap_supervisor_get_free(cap) > pid || pid >= cap_supervisor_get_end(cap))
        return ERROR_INVALID_SUPERVISEE;

    /* Get ptr to supervisee pcb. */
    proc_t* supervisee = &processes[pid];

    /* op(eration) decides what the invocation does */
    switch (op) {
    case ECALL_SUP_SUSPEND: { /* Order suspend of process */
        return proc_supervisor_suspend(supervisee) ? ERROR_OK : ERROR_FAILED;
    }
    case ECALL_SUP_RESUME: { /* Resume process */
        return proc_supervisor_resume(supervisee) ? ERROR_OK : ERROR_FAILED;
    }
    case ECALL_SUP_GET_STATE: { /* Get state */
        current->regs.a1 = supervisee->state;
        return ERROR_OK;
    }
    case ECALL_SUP_READ_REG: { /* Read register */
        if (!proc_supervisor_acquire(supervisee))
            return ERROR_SUPERVISEE_BUSY;
        /* arg0 -> register number */
        current->regs.a1 = proc_read_register(supervisee, arg0);
        proc_supervisor_release(supervisee);
        return ERROR_OK;
    }
    case ECALL_SUP_WRITE_REG: { /* Write register */
        if (!proc_supervisor_acquire(supervisee))
            return ERROR_SUPERVISEE_BUSY;
        /* arg0 -> register number */
        /* arg1 -> value to write */
        proc_write_register(supervisee, arg0, arg1);
        proc_supervisor_release(supervisee);
        return ERROR_OK;
    }
    case ECALL_SUP_READ_CAP: { /* Read capability */
        if (!proc_supervisor_acquire(supervisee))
            return ERROR_SUPERVISEE_BUSY;
        /* arg0 -> cap index to read */
        cap_t cap = cap_node_get_cap(proc_get_cap_node(supervisee, arg0));
        current->regs.a1 = cap.word0;
        current->regs.a2 = cap.word1;
        proc_supervisor_release(supervisee);
        return ERROR_OK;
    }
    case ECALL_SUP_GIVE_CAP: { /* Give capability */
        if (!proc_supervisor_acquire(supervisee))
            return ERROR_SUPERVISEE_BUSY;
        uint64_t code = interprocess_move(current, arg0, supervisee, arg1);
        proc_supervisor_release(supervisee);
        return code;
    }
    case ECALL_SUP_TAKE_CAP: { /* Take capability */
        if (!proc_supervisor_acquire(supervisee))
            return ERROR_SUPERVISEE_BUSY;
        uint64_t code = interprocess_move(supervisee, arg0, current, arg1);
        proc_supervisor_release(supervisee);
        return code;
    }
    default: { /* No matching operation. */
        return ERROR_UNIMPLEMENTED;
    }
    }
}

uint64_t syscall_invoke_receiver(cap_t cap, uint64_t msg0, uint64_t msg1, uint64_t msg2, uint64_t msg3)
{
    kassert(cap_is_type(cap, CAP_TYPE_RECEIVER));
    uint64_t channel = cap_receiver_get_channel(cap);
    proc_receiver_wait(current, channel);
    sched_yield(); /* sched_yield does not return */
}

uint64_t syscall_invoke_sender(cap_t cap, uint64_t msg0, uint64_t msg1, uint64_t msg2, uint64_t msg3, uint64_t src_cidx)
{
    kassert(cap_is_type(cap, CAP_TYPE_SENDER));
    uint64_t channel = cap_sender_get_channel(cap);
    proc_t* receiver = receivers[channel][0];
    if (receiver == NULL || !proc_sender_acquire(receiver, channel))
        return ERROR_NO_RECEIVER;
    if (src_cidx < N_CAPS && receiver->regs.dest_cidx < N_CAPS)
        interprocess_move(current, src_cidx, receiver, receiver->regs.dest_cidx);
    receiver->regs.a0 = ERROR_OK;
    receiver->regs.a1 = msg0;
    receiver->regs.a2 = msg1;
    receiver->regs.a3 = msg2;
    receiver->regs.a4 = msg3;
    proc_sender_release(receiver);
    return ERROR_OK;
}

uint64_t syscall_invoke_server(cap_t cap, uint64_t msg0, uint64_t msg1, uint64_t msg2, uint64_t msg3, uint64_t src_cidx,
                               uint64_t do_receive)
{
    kassert(cap_is_type(cap, CAP_TYPE_SERVER));
    uint64_t channel = cap_server_get_channel(cap);
    /* Get a client waiting on reply */
    proc_t* client = receivers[channel][1];
    if (client != NULL && proc_sender_acquire(client, channel)) {
        if (src_cidx < N_CAPS && client->regs.dest_cidx < N_CAPS)
            interprocess_move(current, src_cidx, client, client->regs.dest_cidx);
        client->regs.a0 = ERROR_OK;
        client->regs.a1 = msg0;
        client->regs.a2 = msg1;
        client->regs.a3 = msg2;
        client->regs.a4 = msg3;
        proc_sender_release(client);
    }

    /* Place the thread in waiting at channel */
    proc_receiver_wait(current, channel);
    /* Yield */
    sched_yield();
}

uint64_t syscall_invoke_client(cap_t cap, uint64_t msg0, uint64_t msg1, uint64_t msg2, uint64_t msg3, uint64_t src_cidx)
{
    kassert(cap_is_type(cap, CAP_TYPE_CLIENT));
    uint64_t channel = cap_client_get_channel(cap);
    proc_t* server = receivers[channel][0];
    if (server == NULL || !proc_sender_acquire(server, channel))
        return ERROR_NO_RECEIVER;

    if (src_cidx < N_CAPS && server->regs.dest_cidx < N_CAPS)
        interprocess_move(current, src_cidx, server, server->regs.dest_cidx);
    server->regs.a0 = ERROR_OK;
    server->regs.a1 = msg0;
    server->regs.a2 = msg1;
    server->regs.a3 = msg2;
    server->regs.a4 = msg3;

    /* Subscribe to replies in channel */
    compare_and_swap(&receivers[channel][1], NULL, current);
    /* Place the thread in waiting at channel */
    proc_receiver_wait(current, channel);
    /* If the thread is not waiting, it was interrupted */
    current->regs.a0 = ERROR_INTERRUPTED;
    /* Release the server */
    proc_sender_release(server);
    /* Yield */
    sched_yield();
}

uint64_t syscall_get_pid(void)
{
    return current->pid;
}

uint64_t syscall_read_reg(uint64_t regnr)
{
    return proc_read_register(current, regnr);
}

uint64_t syscall_write_reg(uint64_t regnr, uint64_t val)
{
    return proc_write_register(current, regnr, val);
}

void syscall_yield(void)
{
    current->regs.timeout = read_timeout(read_csr(mhartid));
    current->regs.a0 = ERROR_OK;
    sched_yield();
}

/*** INTERNAL FUNCTIONS ***/

uint64_t interprocess_move(proc_t* src_proc, uint64_t src_cidx, proc_t* dest_proc, uint64_t dest_cidx)
{
    cap_node_t* src_node = proc_get_cap_node(src_proc, src_cidx);
    cap_t cap = cap_node_get_cap(src_node);
    cap_node_t* dest_node = proc_get_cap_node(dest_proc, dest_cidx);
    if (cap_node_is_deleted(src_node))
        return ERROR_EMPTY;
    if (!cap_node_is_deleted(dest_node))
        return ERROR_COLLISION;
    cap_update_hook(dest_proc, src_node, cap);
    return cap_node_move(cap, src_node, dest_node) ? ERROR_OK : ERROR_EMPTY;
}

void cap_update_hook(proc_t* proc, cap_node_t* node, cap_t cap)
{
    if (cap_is_type(cap, CAP_TYPE_TIME)) {
        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t free = cap_time_get_free(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t pid = (proc != NULL) ? proc->pid : INVALID_PID;
        sched_update(node, hartid, free, end, pid);
    }
    if (cap_is_type(cap, CAP_TYPE_RECEIVER)) {
        uint64_t channel = cap_receiver_get_channel(cap);
        lock_acquire(&receivers_lock);
        if (!cap_node_is_deleted(node)) {
            receivers[channel][0] = proc;
            if (proc == NULL)
                receivers[channel][1] = NULL;
        }
        lock_release(&receivers_lock);
    }
    if (cap_is_type(cap, CAP_TYPE_SERVER)) {
        uint64_t channel = cap_server_get_channel(cap);
        lock_acquire(&receivers_lock);
        if (!cap_node_is_deleted(node)) {
            receivers[channel][0] = proc;
            if (proc == NULL)
                receivers[channel][1] = NULL;
        }
        lock_release(&receivers_lock);
    }
}

cap_t revoke_update_cap(cap_t cap)
{
    switch (cap_get_type(cap)) {
    case CAP_TYPE_MEMORY:
        return cap_memory_set_pmp(cap_memory_set_free(cap, cap_memory_get_begin(cap)), 0);
    case CAP_TYPE_TIME:
        return cap_time_set_free(cap, cap_time_get_begin(cap));
    case CAP_TYPE_CHANNELS:
        return cap_time_set_free(cap, cap_time_get_begin(cap));
    case CAP_TYPE_SUPERVISOR:
        return cap_supervisor_set_free(cap, cap_supervisor_get_begin(cap));
    default:
        return cap;
    }
}

cap_t derive_update_cap(cap_t src_cap, cap_t new_cap)
{
    switch (cap_get_type(src_cap)) {
    case CAP_TYPE_MEMORY:
        if (cap_is_type(new_cap, CAP_TYPE_MEMORY))
            return cap_memory_set_free(src_cap, cap_memory_get_end(new_cap));
        else
            return cap_memory_set_pmp(src_cap, 1);
    case CAP_TYPE_TIME:
        return cap_time_set_free(src_cap, cap_time_get_end(new_cap));
    case CAP_TYPE_CHANNELS:
        if (cap_is_type(new_cap, CAP_TYPE_CHANNELS))
            return cap_channels_set_free(src_cap, cap_channels_get_end(new_cap));
        else if (cap_is_type(new_cap, CAP_TYPE_RECEIVER))
            return cap_channels_set_free(src_cap, cap_receiver_get_channel(new_cap) + 1);
        else if (cap_is_type(new_cap, CAP_TYPE_SERVER))
            return cap_channels_set_free(src_cap, cap_server_get_channel(new_cap) + 1);
        else
            kassert(0);
    case CAP_TYPE_RECEIVER:
        return src_cap;
    case CAP_TYPE_SERVER:
        return src_cap;
    case CAP_TYPE_SUPERVISOR:
        return cap_supervisor_set_free(src_cap, cap_supervisor_get_end(new_cap));
    default:
        kassert(0);
    }
}
