// See LICENSE file for copyright and license details.
#include "syscall_supervisor.h"

#include "interprocess_move.h"
#include "preemption.h"
#include "proc_state.h"
#include "s3k_consts.h"
#include "utils.h"

static void syscall_supervisor_suspend(cap_node_t* cn, cap_t cap, proc_t* supervisee)
    __attribute__((noreturn));
static void syscall_supervisor_resume(cap_node_t* cn, cap_t cap, proc_t* supervisee)
    __attribute__((noreturn));
static void syscall_supervisor_get_state(cap_node_t* cn, cap_t cap, proc_t* supervisee)
    __attribute__((noreturn));
static void syscall_supervisor_read_reg(cap_node_t* cn, cap_t cap, proc_t* supervisee)
    __attribute__((noreturn));
static void syscall_supervisor_write_reg(cap_node_t* cn, cap_t cap, proc_t* supervisee)
    __attribute__((noreturn));
static void syscall_supervisor_read_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
    __attribute__((noreturn));
static void syscall_supervisor_give_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
    __attribute__((noreturn));
static void syscall_supervisor_take_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
    __attribute__((noreturn));

void syscall_supervisor_revoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);
        cap_node_revoke(cn, cap, cap_is_child_supervisor);
        cap_supervisor_set_free(&cap, cap_supervisor_get_begin(cap));
        preemption_disable();
        cap_node_update(cap, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_supervisor_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);
        if (!cap_can_derive_supervisor(cap, newcap))
                trap_syscall_exit(S3K_ILLEGAL_DERIVATION);

        if (cap_get_type(newcap) == CAP_TYPE_SUPERVISOR)
                cap_supervisor_set_free(&cap, cap_supervisor_get_end(newcap));

        preemption_disable();
        cap_node_update(cap, cn);
        cap_node_insert(newcap, newcn, cn);
        trap_syscall_exit2(S3K_OK);
}

void syscall_supervisor_suspend(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        current->regs.a0 = proc_supervisor_suspend(supervisee);
        trap_syscall_exit3();
}

void syscall_supervisor_resume(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        current->regs.a0 = proc_supervisor_resume(supervisee);
        trap_syscall_exit3();
}

void syscall_supervisor_get_state(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        current->regs.a1 = supervisee->state;
        trap_syscall_exit2(S3K_OK);
}

void syscall_supervisor_read_reg(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                current->regs.a1 = proc_read_register(supervisee, current->regs.a3);
                proc_supervisor_release(supervisee);
                trap_syscall_exit2(S3K_OK);
        }
        trap_syscall_exit2(S3K_SUPERVISEE_BUSY);
}

void syscall_supervisor_write_reg(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                proc_write_register(supervisee, current->regs.a3, current->regs.a4);
                proc_supervisor_release(supervisee);
                trap_syscall_exit2(S3K_OK);
        }
        trap_syscall_exit2(S3K_SUPERVISEE_BUSY);
}

void syscall_supervisor_read_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        cap_node_t* supervisee_cap_node = proc_get_cap_node(supervisee, current->regs.a3);
        if (!supervisee_cap_node)
                trap_syscall_exit(S3K_ERROR);
        cap_t supervisee_cap = cap_node_get_cap(supervisee_cap_node);
        preemption_disable();
        current->regs.a1 = supervisee_cap.word0;
        current->regs.a2 = supervisee_cap.word1;
        trap_syscall_exit2(S3K_OK);
}

void syscall_supervisor_give_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                uint64_t code =
                    interprocess_move(current, supervisee, current->regs.a3, current->regs.a4);
                proc_supervisor_release(supervisee);
                trap_syscall_exit2(code);
        }
        trap_syscall_exit2(S3K_SUPERVISEE_BUSY);
}

void syscall_supervisor_take_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                uint64_t code =
                    interprocess_move(supervisee, current, current->regs.a3, current->regs.a4);
                proc_supervisor_release(supervisee);
                trap_syscall_exit2(code);
        }
        trap_syscall_exit2(S3K_SUPERVISEE_BUSY);
}

void syscall_supervisor_invoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);
        uint64_t pid = current->regs.a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap)) {
                trap_syscall_exit(S3K_ERROR);
        }
        proc_t* supervisee = &processes[pid];
        switch (current->regs.a2) {
        case 0:
                syscall_supervisor_suspend(cn, cap, supervisee);
        case 1:
                syscall_supervisor_resume(cn, cap, supervisee);
        case 2:
                syscall_supervisor_get_state(cn, cap, supervisee);
        case 3:
                syscall_supervisor_read_reg(cn, cap, supervisee);
        case 4:
                syscall_supervisor_write_reg(cn, cap, supervisee);
        case 5:
                syscall_supervisor_read_cap(cn, cap, supervisee);
        case 6:
                syscall_supervisor_give_cap(cn, cap, supervisee);
        case 7:
                syscall_supervisor_take_cap(cn, cap, supervisee);
        default:
                trap_syscall_exit(S3K_ERROR);
        }
}
