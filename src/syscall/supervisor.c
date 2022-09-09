#include "proc.h"
#include "cap_node.h"
/* Supervisor operations */
static uint64_t supervisor_suspend(proc_t* supervisee);
static uint64_t supervisor_resume(proc_t* supervisee);
static uint64_t supervisor_get_state(proc_t* supervisee);
static uint64_t supervisor_read_reg(proc_t* supervisee, uint64_t regnr);
static uint64_t supervisor_write_reg(proc_t* supervisee, uint64_t regnr, uint64_t val);
static uint64_t supervisor_read_cap(proc_t* supervisee, uint64_t cidx);
static uint64_t supervisor_give_cap(proc_t* supervisee, uint64_t src_cidx, uint64_t dest_cidx);
static uint64_t supervisor_take_cap(proc_t* supervisee, uint64_t src_cidx, uint64_t dest_cidx);

uint64_t supervisor_suspend(proc_t* supervisee)
{
    return proc_supervisor_suspend(supervisee) ? S3K_OK : S3K_FAILED;
}

uint64_t supervisor_resume(proc_t* supervisee)
{
    return proc_supervisor_resume(supervisee) ? S3K_OK : S3K_FAILED;
}

uint64_t supervisor_get_state(proc_t* supervisee)
{
    current->regs.a1 = supervisee->state;
    return S3K_OK;
}

uint64_t supervisor_read_reg(proc_t* supervisee, uint64_t regnr)
{
    preemption_disable();
    if (proc_supervisor_acquire(supervisee)) {
        current->regs.a1 = proc_read_register(supervisee, regnr);
        proc_supervisor_release(supervisee);
        return S3K_OK;
    }
    return S3K_SUPERVISEE_BUSY;
}

uint64_t supervisor_write_reg(proc_t* supervisee, uint64_t regnr, uint64_t val)
{
    preemption_disable();
    if (proc_supervisor_acquire(supervisee)) {
        proc_write_register(supervisee, regnr, val);
        proc_supervisor_release(supervisee);
        return S3K_OK;
    }
    return S3K_SUPERVISEE_BUSY;
}

uint64_t supervisor_read_cap(proc_t* supervisee, uint64_t cidx)
{
    cap_node_t* supervisee_cap_node = proc_get_cap_node(supervisee, cidx);
    cap_t supervisee_cap = cap_node_get_cap(supervisee_cap_node);
    current->regs.a1 = supervisee_cap.word0;
    current->regs.a2 = supervisee_cap.word1;
    return S3K_OK;
}

uint64_t supervisor_give_cap(proc_t* supervisee, uint64_t src_cidx, uint64_t dest_cidx)
{
    preemption_disable();
    if (proc_supervisor_acquire(supervisee)) {
        uint64_t code = interprocess_move(current, src_cidx, supervisee, dest_cidx);
        proc_supervisor_release(supervisee);
        return code;
    }
    return S3K_SUPERVISEE_BUSY;
}

uint64_t supervisor_take_cap(proc_t* supervisee, uint64_t src_cidx, uint64_t dest_cidx)
{
    preemption_disable();
    if (proc_supervisor_acquire(supervisee)) {
        uint64_t code = interprocess_move(supervisee, src_cidx, current, dest_cidx);
        proc_supervisor_release(supervisee);
        return code;
    }
    return S3K_SUPERVISEE_BUSY;
}
