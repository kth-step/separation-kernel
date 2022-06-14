// See LICENSE file for copyright and license details.

#include "syscall.h"
#include "syscall_nr.h"

static uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                           uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7);
static uint64_t SyscallNoCap(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                             uint64_t a5, uint64_t a6, uint64_t a7);

uint64_t syscall_read(uint64_t cid) {
        if (cid >= 256)
                return -1;
        const Cap cap = cn_get(&current->cap_table[cid]);
        current->args[1] = cap.word0;
        current->args[2] = cap.word1;
        return cap.word0 != 0;
}

uint64_t syscall_delete(uint64_t cid) {
        if (cid >= 256)
                return -1;
        CapNode *cn = &current->cap_table[cid];
        const Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_PMP:
                        CapRevoke(cn);
                        break;
                case CAP_TIME:
                        /* Fix time */
                        break;
                default:
                        break;
        }
        return CapDelete(cn);
}

uint64_t syscall_revoke(uint64_t cid) {
        if (cid >= 256)
                return -1;
        CapNode *cn = &current->cap_table[cid];
        const Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_TIME:
                        /* Fix time */
                        break;
                default:
                        break;
        }
        return CapRevoke(cn);
}

uint64_t syscall_move(uint64_t cid, uint64_t dest) {
        if (cid >= 256 || dest >= 256 || cid == dest)
                return -1;
        CapNode *cn = &current->cap_table[cid];
        CapNode *cn_dest = &current->cap_table[dest];
        return CapMove(cn, cn_dest);
}

uint64_t syscall_derive(uint64_t cid, uint64_t dest, uint64_t word0, uint64_t word1) {
        if (cid >= 256 || dest >= 256 || cid == dest)
                return -1;
        return 0;
}

uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
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
                        return 0;
                default:
                        return -1;
        }
}

uint64_t SyscallNoCap(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
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
