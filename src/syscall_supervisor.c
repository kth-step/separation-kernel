#include "syscall_supervisor.h"

#include "interprocess_move.h"
#include "preemption.h"
#include "proc_state.h"
#include "s3k_consts.h"
#include "utils.h"

static void syscall_supervisor_suspend(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        current->regs.a0 = proc_supervisor_suspend(supervisee);
        trap_syscall_exit3();
}

static void syscall_supervisor_resume(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        current->regs.a0 = proc_supervisor_resume(supervisee);
        trap_syscall_exit3();
}

static void syscall_supervisor_get_state(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        current->regs.a1 = supervisee->state;
        trap_syscall_exit2(S3K_OK);
}

static void syscall_supervisor_read_reg(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                current->regs.a1 = proc_read_register(supervisee, current->regs.a3);
                proc_supervisor_release(supervisee);
                trap_syscall_exit2(S3K_OK);
        }
        trap_syscall_exit2(S3K_SUPERVISEE_BUSY);
}

static void syscall_supervisor_write_reg(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                proc_write_register(supervisee, current->regs.a3, current->regs.a4);
                proc_supervisor_release(supervisee);
                trap_syscall_exit2(S3K_OK);
        }
        trap_syscall_exit2(S3K_SUPERVISEE_BUSY);
}

static void syscall_supervisor_read_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                cap_t supervisee_cap = proc_get_cap(supervisee, current->regs.a3);
                proc_supervisor_release(supervisee);
                current->regs.a1 = supervisee_cap.word0;
                current->regs.a2 = supervisee_cap.word1;
                trap_syscall_exit2(S3K_OK);
        }
        trap_syscall_exit2(S3K_SUPERVISEE_BUSY);
}

static void syscall_supervisor_give_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
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

static void syscall_supervisor_take_cap(cap_node_t* cn, cap_t cap, proc_t* supervisee)
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
