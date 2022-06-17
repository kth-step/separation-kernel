// See LICENSE file for copyright and license details.

#include "syscall.h"

#include "sched.h"
#include "syscall_nr.h"

#define BITWISE_SUBSET(p, q) (((p) & (q)) == (p))

static uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                           uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7);
static uint64_t SyscallNoCap(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                             uint64_t a5, uint64_t a6, uint64_t a7);

static uint64_t syscall_read(uint64_t cid) {
        if (cid >= 256)
                return -1;
        const Cap cap = cn_get(&current->cap_table[cid]);
        current->args[1] = cap.word0;
        current->args[2] = cap.word1;
        return cap.word0 != 0;
}

static uint64_t syscall_delete(uint64_t cid) {
        if (cid >= 256)
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
        if (cid >= 256)
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
        if (cid >= 256 || dest >= 256 || cid == dest)
                return -1;
        CapNode *cn = &current->cap_table[cid];
        CapNode *cn_dest = &current->cap_table[dest];
        return CapMove(cn, cn_dest);
}

static uint64_t syscall_derive_ms(CapNode *cn, CapNode *cn_dest, Cap cap,
                                  Cap cap_new) {
        ASSERT(cap_get_type(cap) == CAP_MEMORY);
        if (cap_get_type(cap_new) == CAP_MEMORY &&
            cap_can_derive_ms_ms(cap, cap_new)) {
                uint64_t end = cap_memory_begin(cap_new);
                cap_memory_set_free(&cap, end);
                return CapUpdate(cap, cn) && CapInsert(cap_new, cn_dest, cn);
        }
        if (cap_get_type(cap_new) == CAP_PMP &&
            cap_can_derive_ms_pe(cap, cap_new)) {
                cap_memory_set_pmp(&cap, 1);
                return CapUpdate(cap, cn) && CapInsert(cap_new, cn_dest, cn);
        }
        return -1;
}

static uint64_t syscall_derive_ts(CapNode *cn, CapNode *cn_dest, Cap cap,
                                  Cap cap_new) {
        ASSERT(cap_get_type(cap) == CAP_TIME);
        if (cap_get_type(cap_new) == CAP_TIME &&
            cap_can_derive_ts_ts(cap, cap_new)) {
                uint64_t end = cap_time_end(cap_new);
                uint64_t id_end = cap_time_id_end(cap_new);
                cap_time_set_free(&cap, end);
                cap_time_set_id_free(&cap, id_end);
                if (CapUpdate(cap, cn) && CapInsert(cap_new, cn_dest, cn)) {
                        SchedUpdate(cap_new, cap, cn);
                        return true;
                }
                return false;
        }
        return -1;
}

static uint64_t syscall_derive_ch(CapNode *cn, CapNode *cn_dest, Cap cap,
                                  Cap cap_new) {
        ASSERT(cap_get_type(cap) == CAP_CHANNELS);
        if (cap_get_type(cap_new) == CAP_CHANNELS &&
            cap_can_derive_ch_ch(cap, cap_new)) {
                uint64_t end = cap_channels_end(cap_new);
                cap_channels_set_free(&cap, end);
                return CapUpdate(cap, cn) && CapInsert(cap_new, cn_dest, cn);
        }
        if (cap_get_type(cap_new) == CAP_ENDPOINT &&
            cap_can_derive_ch_ep(cap, cap_new)) {
                cap_channels_set_ep(&cap, 1);
                return CapUpdate(cap, cn) && CapInsert(cap_new, cn_dest, cn);
        }
        return -1;
}

static uint64_t syscall_derive_su(CapNode *cn, CapNode *cn_dest, Cap cap,
                                  Cap cap_new) {
        ASSERT(cap_get_type(cap) == CAP_SUPERVISOR);
        if (cap_get_type(cap_new) == CAP_SUPERVISOR &&
            cap_can_derive_su_su(cap, cap_new)) {
                uint64_t end = cap_supervisor_end(cap_new);
                cap_supervisor_set_free(&cap, end);
                return CapUpdate(cap, cn) && CapInsert(cap_new, cn_dest, cn);
        }
        return -1;
}

static uint64_t syscall_derive(uint64_t cid, uint64_t dest, uint64_t word0,
                               uint64_t word1) {
        if (cid >= 256 || dest >= 256 || cid == dest)
                return -1;
        CapNode *cn = &current->cap_table[cid];
        CapNode *cn_dest = &current->cap_table[dest];
        const Cap cap = cn_get(cn);
        const Cap cap_dest = (Cap){word0, word1};
        switch (cap_get_type(cap)) {
                case CAP_MEMORY:
                        return syscall_derive_ms(cn, cn_dest, cap, cap_dest);
                case CAP_TIME:
                        return syscall_derive_ts(cn, cn_dest, cap, cap_dest);
                case CAP_CHANNELS:
                        return syscall_derive_ch(cn, cn_dest, cap, cap_dest);
                case CAP_SUPERVISOR:
                        return syscall_derive_su(cn, cn_dest, cap, cap_dest);
                default:
                        return -1;
        }
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

static uint64_t syscall_invoke_supervisor(Cap cap, CapNode *cn, uint64_t a1,
                                          uint64_t a2) {
        return -1;
}

static uint64_t syscall_invoke(uint64_t a0, uint64_t a1, uint64_t a2,
                               uint64_t a3, uint64_t a4, uint64_t a5,
                               uint64_t a6) {
        if (a0 >= 256)
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
                        return syscall_invoke_supervisor(cap, cn, a1, a2);
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
