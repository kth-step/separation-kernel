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

/* Function declarations */
static void syscall_handle_0(registers_t* regs);
static void syscall_unimplemented(registers_t* regs, cap_node_t* cn, cap_t cap);
static void syscall_read_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
static void syscall_move_cap(registers_t* regs, cap_node_t* cn, cap_t cap);
static void syscall_delete_cap(registers_t* regs, cap_node_t* cn, cap_t cap);

typedef void (*handler_t)(registers_t*, cap_node_t*, cap_t);

static handler_t cap_handlers[NUM_OF_CAP_TYPES][NUM_OF_CAP_SYSNR] = {
    /* EMPTY */
    [CAP_TYPE_EMPTY] =
        {
            [S3K_SYSNR_READ_CAP] = syscall_read_cap,
        },
    /* MEMORY */
    [CAP_TYPE_MEMORY] =
        {
            [S3K_SYSNR_READ_CAP] = syscall_read_cap,
            [S3K_SYSNR_MOVE_CAP] = syscall_move_cap,
            [S3K_SYSNR_DELETE_CAP] = syscall_delete_cap,
            [S3K_SYSNR_REVOKE_CAP] = syscall_memory_revoke_cap,
            [S3K_SYSNR_DERIVE_CAP] = syscall_memory_derive_cap,
        },
    [CAP_TYPE_PMP] =
        {
            [S3K_SYSNR_READ_CAP] = syscall_read_cap,
            [S3K_SYSNR_MOVE_CAP] = syscall_move_cap,
            [S3K_SYSNR_DELETE_CAP] = syscall_delete_cap,
        },
    /* TIME */
    [CAP_TYPE_TIME] =
        {
            [S3K_SYSNR_READ_CAP] = syscall_read_cap,
            [S3K_SYSNR_MOVE_CAP] = syscall_move_cap,
            [S3K_SYSNR_DELETE_CAP] = syscall_time_delete_cap,
            [S3K_SYSNR_REVOKE_CAP] = syscall_time_revoke_cap,
            [S3K_SYSNR_DERIVE_CAP] = syscall_time_derive_cap,
        },
    /* IPC */
    [CAP_TYPE_CHANNELS] =
        {
            [S3K_SYSNR_READ_CAP] = syscall_read_cap,
            [S3K_SYSNR_MOVE_CAP] = syscall_move_cap,
            [S3K_SYSNR_DELETE_CAP] = syscall_delete_cap,
            [S3K_SYSNR_REVOKE_CAP] = syscall_channels_revoke_cap,
            [S3K_SYSNR_DERIVE_CAP] = syscall_channels_derive_cap,
        },
    [CAP_TYPE_RECEIVER] =
        {
            [S3K_SYSNR_READ_CAP] = syscall_read_cap,
            [S3K_SYSNR_MOVE_CAP] = syscall_move_cap,
            [S3K_SYSNR_DELETE_CAP] = syscall_delete_cap,
            [S3K_SYSNR_REVOKE_CAP] = syscall_receiver_revoke_cap,
            [S3K_SYSNR_DERIVE_CAP] = syscall_receiver_derive_cap,
            [S3K_SYSNR_RECEIVE] = syscall_receiver_receive,
        },
    [CAP_TYPE_SENDER] =
        {
            [S3K_SYSNR_READ_CAP] = syscall_read_cap,
            [S3K_SYSNR_MOVE_CAP] = syscall_move_cap,
            [S3K_SYSNR_DELETE_CAP] = syscall_delete_cap,
            [S3K_SYSNR_SEND] = syscall_sender_send,
        },
    /* SUPERVISOR */
    [CAP_TYPE_SUPERVISOR] =
        {
            [S3K_SYSNR_READ_CAP] = syscall_read_cap,
            [S3K_SYSNR_MOVE_CAP] = syscall_move_cap,
            [S3K_SYSNR_DELETE_CAP] = syscall_delete_cap,
            [S3K_SYSNR_SUPERVISOR_SUSPEND] = syscall_supervisor_suspend,
            [S3K_SYSNR_SUPERVISOR_RESUME] = syscall_supervisor_resume,
            [S3K_SYSNR_SUPERVISOR_GET_STATE] = syscall_supervisor_get_state,
            [S3K_SYSNR_SUPERVISOR_READ_REG] = syscall_supervisor_read_reg,
            [S3K_SYSNR_SUPERVISOR_WRITE_REG] = syscall_supervisor_write_reg,
            [S3K_SYSNR_SUPERVISOR_READ_CAP] = syscall_supervisor_read_cap,
            [S3K_SYSNR_SUPERVISOR_GIVE_CAP] = syscall_supervisor_give_cap,
            [S3K_SYSNR_SUPERVISOR_TAKE_CAP] = syscall_supervisor_take_cap,
        },
};

void syscall_init(void)
{
        for (int i = 0; i < NUM_OF_CAP_TYPES; ++i) {
                for (int j = 0; j < NUM_OF_CAP_SYSNR; ++j) {
                        if (cap_handlers[i][j] == NULL)
                                cap_handlers[i][j] = syscall_unimplemented;
                }
        }
}

void syscall_handler(registers_t* regs)
{
        int64_t syscall_nr = regs->a7;
        if (syscall_nr < 0) {
                syscall_handle_0(regs);
        } else if (syscall_nr < NUM_OF_CAP_SYSNR) {
                cap_node_t* cn = proc_get_cap_node(current, regs->a0);
                cap_t cap = cap_node_get_cap(cn);
                cap_handlers[cap_get_type(cap)][syscall_nr](regs, cn, cap);
        } else {
                exception_handler(regs, 8, 0);
        }
}

void syscall_unimplemented(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        preemption_disable();
        regs->a0 = S3K_UNIMPLEMENTED;
        regs->pc += 4;
        kprint("UNIMPLEMENTED\n");
        preemption_disable();
}

void syscall_read_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        preemption_disable();
        regs->a0 = cap.word0;
        regs->a1 = cap.word1;
        regs->pc += 4;
        preemption_enable();
}

void syscall_move_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        preemption_disable();
        cap_node_t* cndest = proc_get_cap_node(current, regs->a1);
        if (!cap_node_is_deleted(cndest)) {
                regs->a0 = S3K_COLLISION;
        } else {
                regs->a0 = cap_node_move(cn, cndest) ? S3K_OK : S3K_ERROR;
        }
        regs->pc += 4;
        preemption_enable();
}

void syscall_delete_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        preemption_disable();
        regs->a0 = cap_node_delete(cn) ? S3K_OK : S3K_ERROR;
        regs->pc += 4;
        preemption_enable();
}

void syscall_handle_0(registers_t* regs)
{
        preemption_disable();
        switch (regs->a7) {
                case S3K_SYSNR_GET_PID:
                        /* Get the process ID */
                        regs->a0 = current->pid;
                        regs->pc += 4;
                        break;
                case S3K_SYSNR_READ_REGISTER:
                        regs->a0 = proc_read_register(current, regs->a0);
                        regs->pc += 4;
                        break;
                case S3K_SYSNR_WRITE_REGISTER:
                        break;
                        regs->a0 = proc_write_register(current, regs->a0, regs->a1);
                        regs->pc += 4;
                        break;
                case S3K_SYSNR_YIELD:
                        regs->timeout = read_timeout(read_csr(mhartid));
                        regs->pc += 4;
                        trap_yield();
                        break;
                default:
                        regs->a0 = S3K_UNIMPLEMENTED;
                        regs->pc += 4;
                        break;
        }
        preemption_enable();
}
