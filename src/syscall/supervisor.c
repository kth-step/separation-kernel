// See LICENSE file for copyright and license details.
#include "syscall.h"
static uint64_t sup_halt(Proc *supervisee) {
        /* Check if supervisee has halted or is to halt */
        if (supervisee->state == PROC_HALTED || supervisee->halt)
                return false;
        ProcHalt(supervisee);
        return true;
}

static uint64_t sup_is_halted(Proc *supervisee) {
        return supervisee->state == PROC_HALTED;
}

static uint64_t sup_resume(Proc *supervisee) {
        if (supervisee->state == PROC_HALTED) {
                supervisee->state = PROC_SUSPENDED;
                return true;
        }
        return false;
}

static uint64_t sup_reset(Proc *supervisee, CapNode *cn) {
        Cap cap = cn_get(cn);
        if (cap_get_type(cap) != CAP_TYPE_PMP_ENTRY)
                return -1;
        if (supervisee->state != PROC_HALTED)
                return -1;
        /* Reset the process */
        ProcReset(supervisee->pid);
        return CapMove(proc_get_cn(supervisee, 0), cn) &&
               ProcLoadPmp(supervisee, cap, cn, 0);
}

static uint64_t sup_read(Proc *supervisee, uint64_t cid) {
        Cap cap = cn_get(proc_get_cn(supervisee, cid));
        current->args[1] = cap.word1;
        return cap.word0;
}

static uint64_t sup_give(Proc *supervisee, uint64_t cid_dest,
                         uint64_t cid_src) {
        return 0;
}

static uint64_t sup_take(Proc *supervisee, uint64_t cid_dest,
                         uint64_t cid_src) {
        return 0;
}

uint64_t SyscallSupervisor(const Cap cap, CapNode *cn, uint64_t a1, uint64_t a2,
                           uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6,
                           uint64_t a7) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);
        Proc *supervisee = &processes[cap_supervisor_pid(cap)];
        switch (a7) {
                case SYSNR_READ_CAP:
                        /* Read cap */
                        current->args[1] = cap.word0;
                        current->args[2] = cap.word1;
                        return 1;
                case SYSNR_MOVE_CAP:
                        /* Move cap */
                        return CapMove(curr_get_cn(a1), cn);
                case SYSNR_DELETE_CAP:
                        /* Delete time slice */
                        return CapDelete(cn);
                case SYSNR_SP_IS_HALTED:
                        return sup_is_halted(supervisee);
                case SYSNR_SP_HALT:
                        /* Halt process */
                        return sup_halt(supervisee);
                case SYSNR_SP_RESUME:
                        /* Resume process */
                        return sup_resume(supervisee);
                case SYSNR_SP_RESET:
                        /* Reset process */
                        return sup_reset(supervisee, curr_get_cn(a1));
                case SYSNR_SP_READ:
                        /* Read cap */
                        return sup_read(supervisee, a1);
                case SYSNR_SP_GIVE:
                        /* Give cap */
                        return sup_give(supervisee, a1, a2);
                case SYSNR_SP_TAKE:
                        /* Take cap */
                        return sup_take(supervisee, a1, a2);
                default:
                        return -1;
        }
}
