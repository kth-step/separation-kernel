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

/* Function declarations */
static void syscall_handle_0(registers_t* regs);
static void syscall_handle_empty(registers_t* regs, cap_node_t* cn, cap_t cap);

static void (*const syscall_handler_array[])(registers_t*, cap_node_t* cn, cap_t cap) = {
    /* EMPTY */
    [CAP_TYPE_EMPTY] = syscall_handle_empty,
    /* MEMORY */
    [CAP_TYPE_MEMORY] = syscall_handle_memory,
    [CAP_TYPE_PMP] = syscall_handle_pmp,
    /* TIME */
    [CAP_TYPE_TIME] = syscall_handle_time,
    /* IPC */
    [CAP_TYPE_CHANNELS] = syscall_handle_channels,
    [CAP_TYPE_RECEIVER] = syscall_handle_receiver,
    [CAP_TYPE_SENDER] = syscall_handle_sender,
    /* SUPERVISOR */
    [CAP_TYPE_SUPERVISOR] = syscall_handle_supervisor,
};

void syscall_handler(registers_t* regs)
{
        if (regs->a0 == 0) {
                syscall_handle_0(regs);
        } else {
                cap_node_t* cn = proc_get_cap_node(current, regs->a0);
                cap_t cap = cap_node_get_cap(cn);
                syscall_handler_array[cap_get_type(cap)](regs, cn, cap);
        }
}

void syscall_handle_empty(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        preemption_disable();
        regs->a0 = S3K_EMPTY;
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
