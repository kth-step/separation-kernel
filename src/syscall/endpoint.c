// See LICENSE file for copyright and license details.
#include "syscall.h"

uint64_t SyscallEndpoint(const Cap cap, CapNode *cn, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6,
                         uint64_t sysnr) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_ENDPOINT);
        switch (sysnr) {
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
                default:
                        return -1;
        }
}

