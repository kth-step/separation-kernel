#include "syscall_supervisor.h"

#include "interprocess_move.h"
#include "preemption.h"
#include "proc_state.h"
#include "s3k_consts.h"

void syscall_supervisor_derive_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);

        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
        cap_t newcap = (cap_t){regs->a2, regs->a3};

        if (!cap_node_is_deleted(newcn)) {
                preemption_disable();
                regs->a0 = S3K_COLLISION;
                regs->pc += 4;
                preemption_disable();
                return;
        }

        if (!cap_can_derive(cap, newcap)) {
                preemption_disable();
                regs->a0 = S3K_ILLEGAL_DERIVATION;
                regs->pc += 4;
                preemption_disable();
                return;
        }

        kassert(cap_get_type(newcap) == CAP_TYPE_SUPERVISOR);

        cap_supervisor_set_free(&cap, cap_supervisor_get_end(newcap));

        preemption_enable();
        if (cap_node_update(cap, cn) && cap_node_insert(newcap, newcn, cn)) {
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_ERROR;
        }
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_revoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);

        cap_node_revoke(cn);

        cap_supervisor_set_free(&cap, cap_supervisor_get_begin(cap));

        preemption_enable();
        if (cap_node_update(cap, cn))
                regs->a0 = S3K_OK;
        else
                regs->a0 = S3K_ERROR;
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_suspend(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_enable();
        regs->a0 = proc_supervisor_suspend(supervisee);
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_resume(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_enable();
        regs->a0 = proc_supervisor_resume(supervisee);
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_get_state(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_enable();
        regs->a0 = S3K_OK;
        regs->a1 = supervisee->state;
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_read_reg(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_enable();
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = proc_read_register(supervisee, regs->a3);
                proc_supervisor_release(supervisee);
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
        }
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_write_reg(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_enable();
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = proc_write_register(supervisee, regs->a3, regs->a4);
                proc_supervisor_release(supervisee);
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
        }
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_read_cap(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_enable();
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
        preemption_disable();
}

void syscall_supervisor_give_cap(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_enable();
        if (proc_supervisor_acquire(supervisee)) {
                regs->a0 = interprocess_move(current, supervisee, regs->a3, regs->a4);
                proc_supervisor_release(supervisee);
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
        }
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_take_cap(registers_t* regs, cap_node_t* cn, cap_t cap, proc_t* supervisee)
{
        preemption_enable();
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = interprocess_move(supervisee, current, regs->a3, regs->a4);
                proc_supervisor_release(supervisee);
                regs->a0 = S3K_OK;
        } else {
                regs->a0 = S3K_SUPERVISEE_BUSY;
        }
        regs->pc += 4;
        preemption_disable();
}

void syscall_supervisor_invoke_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a2;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap)) {
                preemption_enable();
                regs->a0 = S3K_ERROR;
                regs->pc += 4;
                preemption_disable();
                return;
        }
        proc_t* supervisee = &processes[pid];
        switch (regs->a1) {
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
                        regs->a0 = S3K_UNIMPLEMENTED;
                        regs->pc += 4;
                        preemption_disable();
        }
}
