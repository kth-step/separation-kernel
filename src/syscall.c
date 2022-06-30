// See LICENSE file for copyright and license details.

#include "sched.h"
#include "types.h"
#include "syscall_nr.h"

static uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                           uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7);
static uint64_t SyscallNoCap(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                             uint64_t a5, uint64_t a6, uint64_t a7);

static uint64_t syscall_read(uint64_t cid) {
        if (cid >= N_CAPS)
                return -1;
        const Cap cap = cn_get(&current->cap_table[cid]);
        current->args[1] = cap.word0;
        current->args[2] = cap.word1;
        return cap.word0 != 0;
}

static uint64_t syscall_delete(uint64_t cid) {
        if (cid >= N_CAPS)
                return -1;
        CapNode *cn = &current->cap_table[cid];
        const Cap cap = cn_get(cn);
        //        bool succ;
        switch (cap_get_type(cap)) {
                case CAP_PMP:
                        CapRevoke(cn);
                        return CapDelete(cn);
                case CAP_TIME:
                        /* Unset time here */
                        SchedDelete(cap, cn);
                        return CapDelete(cn);
                default:
                        return CapDelete(cn);
        }
}

static uint64_t syscall_revoke(uint64_t cid) {
        if (cid >= N_CAPS)
                return -1;
        CapNode *cn = &current->cap_table[cid];
        const Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_TIME:
                        SchedRevoke(cap, cn);
                        break;
                default:
                        break;
        }
        return CapRevoke(cn);
}

static uint64_t syscall_move(uint64_t cid, uint64_t dest) {
        if (cid >= N_CAPS || dest >= 256 || cid == dest)
                return -1;
        CapNode *cn = &current->cap_table[cid];
        CapNode *cn_dest = &current->cap_table[dest];
        return CapMove(cn, cn_dest);
}

static uint64_t syscall_derive(uint64_t cid, uint64_t dest, uint64_t word0,
                               uint64_t word1) {
        if (cid >= N_CAPS || dest >= 256 || cid == dest)
                return -1;
        CapNode *cn_parent = &current->cap_table[cid];
        CapNode *cn_child = &current->cap_table[dest];
        const Cap parent = cn_get(cn_parent);
        const Cap child = (Cap){word0, word1};
        CapType parent_type = cap_get_type(parent);
        CapType child_type = cap_get_type(child);
        if (!cap_can_derive(parent, child))
                return -1;
        if (parent_type == CAP_MEMORY && child_type == CAP_MEMORY) {
                uint64_t end = cap_memory_get_begin(child);
                Cap c = cap_memory_set_free(parent, end);
                return CapUpdate(c, cn_parent) &&
                       CapInsert(child, cn_child, cn_parent);
        }
        if (parent_type == CAP_MEMORY && child_type == CAP_PMP) {
                Cap c = cap_memory_set_pmp(parent, 1);
                return CapUpdate(c, cn_parent) &&
                       CapInsert(child, cn_child, cn_parent);
        }
        if (parent_type == CAP_TIME && child_type == CAP_TIME) {
                uint64_t end = cap_time_get_end(child);
                Cap c = cap_time_set_free(parent, end);
                return CapUpdate(c, cn_parent) &&
                       CapInsert(child, cn_child, cn_parent) &&
                       SchedUpdate(child, parent, cn_parent);
        }
        if (parent_type == CAP_CHANNELS && child_type == CAP_CHANNELS) {
                uint64_t end = cap_channels_get_end(child);
                Cap c = cap_channels_set_free(parent, end);
                return CapUpdate(c, cn_parent) && CapInsert(child, cn_child, cn_parent);
        }
        if (parent_type == CAP_CHANNELS && child_type == CAP_ENDPOINT) {
                Cap c = cap_channels_set_ep(parent, 1);
                return CapUpdate(c, cn_parent) && CapInsert(child, cn_child, cn_parent);
        }
        if (parent_type == CAP_SUPERVISOR && child_type == CAP_SUPERVISOR) {
                uint64_t end = cap_supervisor_get_end(child);
                Cap c = cap_supervisor_set_free(parent, end);
                return CapUpdate(c, cn_parent) && CapInsert(child, cn_child, cn_parent);
        }
        kassert(false);
}

static uint64_t syscall_invoke_pmp(Cap cap, CapNode *cn, uint64_t index) {
        if (index >= N_PMP)
                return -1;
        return ProcLoadPmp(current, cap, cn, index);
}

static uint64_t syscall_invoke_endpoint(Cap cap, CapNode *cn, uint64_t a1,
                                        uint64_t a2, uint64_t a3, uint64_t a4,
                                        uint64_t a5, uint64_t a6) {
        return -1;
}

static uint64_t syscall_invoke_supervisor_halt(Proc *supervisee) {
        return -1;
}

static uint64_t syscall_invoke_supervisor_resume(Proc *supervisee) {
        return -1;
}

static uint64_t syscall_invoke_supervisor_reset(Proc *supervisee,
                                                uint64_t cid0) {
        return -1;
}

static uint64_t syscall_invoke_supervisor_give(Proc *supervisee, uint64_t cid0,
                                               uint64_t cid1) {
        return -1;
}

static uint64_t syscall_invoke_supervisor_take(Proc *supervisee, uint64_t cid0,
                                               uint64_t cid1) {
        return -1;
}

static uint64_t syscall_invoke_supervisor(Cap cap, CapNode *cn, uint64_t op,
                                          uint64_t pid, uint64_t cid0,
                                          uint64_t cid1) {
        /* TODO:
         * - HALT
         * - RESUME
         * - RESET
         * - READ_CAP
         * - GIVE_CAP
         * - TAKE_CAP
         */

        /* Check free <= pid < end */
        uint64_t pid_free = cap_supervisor_get_free(cap);
        uint64_t pid_end = cap_supervisor_get_end(cap);
        if (pid < pid_free || pid >= pid_end)
                return -1;

        Proc *supervisee = &processes[pid];
        switch (op) {
                case 0:
                        return syscall_invoke_supervisor_halt(supervisee);
                case 1:
                        return syscall_invoke_supervisor_resume(supervisee);
                case 2:
                        return syscall_invoke_supervisor_resume(supervisee);
                case 3:
                        return syscall_invoke_supervisor_reset(supervisee,
                                                               cid0);
                case 4:
                        return syscall_invoke_supervisor_give(supervisee, cid0,
                                                              cid1);
                case 5:
                        return syscall_invoke_supervisor_take(supervisee, cid0,
                                                              cid1);
                default:
                        return -1;
        }
}

static uint64_t syscall_invoke(uint64_t a0, uint64_t a1, uint64_t a2,
                               uint64_t a3, uint64_t a4, uint64_t a5,
                               uint64_t a6) {
        if (a0 >= N_CAPS)
                return -1;
        CapNode *cn = &current->cap_table[a0];
        Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_PMP:
                        return syscall_invoke_pmp(cap, cn, a1);
                case CAP_ENDPOINT:
                        return syscall_invoke_endpoint(cap, cn, a1, a2, a3, a4,
                                                       a5, a6);
                case CAP_SUPERVISOR:
                        return syscall_invoke_supervisor(cap, cn, a1, a2, a3,
                                                         a4);
                default:
                        return -1;
        }
}

static uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                           uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
        switch (a7) {
                case SYSNR_READ_CAP:
                        return syscall_read(a0);
                case SYSNR_DELETE_CAP:
                        return syscall_delete(a0);
                case SYSNR_REVOKE_CAP:
                        return syscall_revoke(a0);
                case SYSNR_MOVE_CAP:
                        return syscall_move(a0, a1);
                case SYSNR_DERIVE_CAP:
                        return syscall_derive(a0, a1, a2, a3);
                case SYSNR_INVOKE_CAP:
                        return syscall_invoke(a0, a1, a2, a3, a4, a5, a6);
                default:
                        return -1;
        }
}

static uint64_t SyscallNoCap(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                             uint64_t a5, uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        return syscall_read(0);
                case 1:
                        /* Get the Process ID */
                        return current->pid;
                default:
                        return -1;
        }
}

void SyscallHandler(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
        if (a0 == 0) {
                current->args[0] = SyscallNoCap(a1, a2, a3, a4, a5, a6, a7);
        } else {
                current->args[0] = SyscallCap(a0, a1, a2, a3, a4, a5, a6, a7);
        }
}
