#include "syscall_supervisor.h"

#include "interprocess_move.h"
#include "preemption.h"
#include "s3k_consts.h"

static inline uint64_t supervisor_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);

        if (!cap_node_is_deleted(newcn))
                return S3K_COLLISION;

        if (!cap_can_derive(cap, newcap))
                return S3K_ILLEGAL_DERIVATION;

        kassert(cap_get_type(newcap) == CAP_TYPE_SUPERVISOR);

        cap_supervisor_set_free(&cap, cap_supervisor_get_end(newcap));

        if (cap_node_update(cap, cn) && cap_node_insert(newcap, newcn, cn))
                return S3K_OK;

        return S3K_ERROR;
}

static inline uint64_t supervisor_revoke_cap(cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);

        preemption_enable();
        cap_node_revoke(cn);
        preemption_disable();

        cap_supervisor_set_free(&cap, cap_supervisor_get_begin(cap));

        if (cap_node_update(cap, cn))
                return S3K_OK;
        return S3K_ERROR;
}

static inline uint64_t supervisor_suspend(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap))
                return S3K_ERROR;
        proc_t* supervisee = &processes[pid];
        return proc_supervisor_suspend(supervisee);
}

static inline uint64_t supervisor_resume(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap))
                return S3K_ERROR;
        proc_t* supervisee = &processes[pid];
        return proc_supervisor_resume(supervisee);
}

static inline uint64_t supervisor_get_state(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap))
                return S3K_ERROR;
        proc_t* supervisee = &processes[pid];
        regs->a1 = supervisee->state;
        return S3K_OK;
}

static inline uint64_t supervisor_read_reg(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap))
                return S3K_ERROR;
        proc_t* supervisee = &processes[pid];
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = proc_read_register(supervisee, regs->a2);
                proc_supervisor_release(supervisee);
                return S3K_OK;
        }
        return S3K_SUPERVISEE_BUSY;
}

static inline uint64_t supervisor_write_reg(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap))
                return S3K_ERROR;
        proc_t* supervisee = &processes[pid];
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = proc_write_register(supervisee, regs->a2, regs->a3);
                proc_supervisor_release(supervisee);
                return S3K_OK;
        }
        return S3K_SUPERVISEE_BUSY;
}

static inline uint64_t supervisor_read_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap))
                return S3K_ERROR;
        proc_t* supervisee = &processes[pid];
        if (proc_supervisor_acquire(supervisee)) {
                cap_t supervisee_cap = proc_get_cap(supervisee, regs->a2);
                proc_supervisor_release(supervisee);
                regs->a1 = supervisee_cap.word0;
                regs->a2 = supervisee_cap.word0;
                return S3K_OK;
        }
        return S3K_SUPERVISEE_BUSY;
}

static inline uint64_t supervisor_give_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap))
                return S3K_ERROR;
        proc_t* supervisee = &processes[pid];
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = interprocess_move(current, supervisee, regs->a2, regs->a3);
                proc_supervisor_release(supervisee);
                return S3K_OK;
        }
        return S3K_SUPERVISEE_BUSY;
}

static inline uint64_t supervisor_take_cap(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        uint64_t pid = regs->a1;
        if (pid < cap_supervisor_get_free(cap) || pid >= cap_supervisor_get_end(cap))
                return S3K_ERROR;
        proc_t* supervisee = &processes[pid];
        if (proc_supervisor_acquire(supervisee)) {
                regs->a1 = interprocess_move(supervisee, current, regs->a2, regs->a3);
                proc_supervisor_release(supervisee);
                return S3K_OK;
        }
        return S3K_SUPERVISEE_BUSY;
}

void syscall_handle_supervisor(registers_t* regs, cap_node_t* cn, cap_t cap)
{
        kassert(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);
        preemption_disable();
        switch (regs->a7) {
                case S3K_SYSNR_READ_CAP:
                        regs->a0 = S3K_OK;
                        regs->a1 = cap.word0;
                        regs->a2 = cap.word1;
                        break;

                case S3K_SYSNR_MOVE_CAP:
                        regs->a0 = cap_node_move(cn, proc_get_cap_node(current, regs->a1)) ? S3K_OK : S3K_ERROR;
                        break;

                case S3K_SYSNR_DELETE_CAP:
                        regs->a0 = cap_node_delete(cn) ? S3K_OK : S3K_ERROR;
                        break;

                case S3K_SYSNR_REVOKE_CAP:
                        regs->a0 = supervisor_revoke_cap(cn, cap);
                        break;

                case S3K_SYSNR_DERIVE_CAP: {
                        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
                        cap_t newcap = (cap_t){regs->a2, regs->a3};
                        regs->a0 = supervisor_derive_cap(cn, cap, newcn, newcap);
                } break;

                case S3K_SYSNR_SUPERVISOR_SUSPEND:
                        regs->a0 = supervisor_suspend(regs, cn, cap);
                        break;

                case S3K_SYSNR_SUPERVISOR_RESUME:
                        regs->a0 = supervisor_resume(regs, cn, cap);
                        break;

                case S3K_SYSNR_SUPERVISOR_GET_STATE:
                        regs->a0 = supervisor_get_state(regs, cn, cap);
                        break;

                case S3K_SYSNR_SUPERVISOR_READ_REG:
                        regs->a0 = supervisor_read_reg(regs, cn, cap);
                        break;

                case S3K_SYSNR_SUPERVISOR_WRITE_REG:
                        regs->a0 = supervisor_write_reg(regs, cn, cap);
                        break;

                case S3K_SYSNR_SUPERVISOR_READ_CAP:
                        regs->a0 = supervisor_read_cap(regs, cn, cap);
                        break;

                case S3K_SYSNR_SUPERVISOR_GIVE_CAP:
                        regs->a0 = supervisor_give_cap(regs, cn, cap);
                        break;

                case S3K_SYSNR_SUPERVISOR_TAKE_CAP:
                        regs->a0 = supervisor_take_cap(regs, cn, cap);
                        break;

                default:
                        regs->a0 = S3K_UNIMPLEMENTED;
                        break;
        }
        regs->pc += 4;
        preemption_enable();
}
