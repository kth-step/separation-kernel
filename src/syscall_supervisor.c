#include "syscall_supervisor.h"

#include "interprocess_move.h"
#include "preemption.h"
#include "proc_state.h"
#include "s3k_consts.h"
#include "utils.h"

static void syscall_supervisor_suspend(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        regs->a0 = proc_supervisor_suspend(supervisee);
        regs->pc += 4;
}

static void syscall_supervisor_resume(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        regs->a0 = proc_supervisor_resume(supervisee);
        regs->pc += 4;
}

static void syscall_supervisor_get_state(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        regs->a0 = S3K_OK;
        regs->a1 = supervisee->state;
        regs->pc += 4;
}

static void syscall_supervisor_read_reg(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = proc_read_register(supervisee, regs->a3);
                proc_supervisor_release(supervisee);
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
        }
        regs->pc += 4;
}

static void syscall_supervisor_write_reg(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                proc_write_register(supervisee, regs->a3, regs->a4);
                proc_supervisor_release(supervisee);
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
        }
        regs->pc += 4;
}

static void syscall_supervisor_read_cap(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                cap_t supervisee_cap = proc_get_cap(supervisee, regs->a3);
                proc_supervisor_release(supervisee);
                regs->a0 = S3K_OK;
                regs->a1 = supervisee_cap.word0;
                regs->a2 = supervisee_cap.word1;
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
        }
        regs->pc += 4;
}

static void syscall_supervisor_give_cap(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                regs->a0 = interprocess_move(current, supervisee, regs->a3, regs->a4);
                proc_supervisor_release(supervisee);
                regs->pc += 4;
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
                regs->pc += 4;
        }
}

static void syscall_supervisor_take_cap(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_disable();
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = interprocess_move(supervisee, current, regs->a3, regs->a4);
                proc_supervisor_release(supervisee);
                regs->a0 = S3K_OK;
                regs->pc += 4;
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
                regs->pc += 4;
        }
}

void syscall_supervisor_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);
        kassert(regs == &current->regs);
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap)) {
                preemption_disable();
                regs->a0 = S3K_ERROR;
                regs->pc += 4;
                return;
        }
        proc_t* supervisee = &processes[pid];
        switch (regs->a2) {
        case 0:
                syscall_supervisor_suspend(regs, cn, cap, supervisee);
                break;
        case 1:
                syscall_supervisor_resume(regs, cn, cap, supervisee);
                break;
        case 2:
                syscall_supervisor_get_state(regs, cn, cap, supervisee);
                break;
        case 3:
                syscall_supervisor_read_reg(regs, cn, cap, supervisee);
                break;
        case 4:
                syscall_supervisor_write_reg(regs, cn, cap, supervisee);
                break;
        case 5:
                syscall_supervisor_read_cap(regs, cn, cap, supervisee);
                break;
        case 6:
                syscall_supervisor_give_cap(regs, cn, cap, supervisee);
                break;
        case 7:
                syscall_supervisor_take_cap(regs, cn, cap, supervisee);
                break;
        default:
                preemption_enable();
                regs->a0 = S3K_ERROR;
                regs->pc += 4;
                break;
        }
}
