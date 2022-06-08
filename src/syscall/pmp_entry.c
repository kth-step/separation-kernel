// See LICENSE file for copyright and license details.
#include "syscall.h"
uint64_t SyscallPmpEntry(const Cap cap, CapNode *cn, uint64_t a1,
                         uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                         uint64_t a6, uint64_t a7) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_PMP_ENTRY);
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
                        /* Delete cap */
                        /* Revoke unloads PMP by deleting a CapLoadedPmp */
                        CapRevoke(cn);
                        return CapDelete(cn);
                case SYSNR_PE_UNLOAD:
                        /* Unload cap */
                        return CapRevoke(cn);
                case SYSNR_PE_LOAD:
                        /* Load cap */
                        return ProcLoadPmp(current, cap, cn, a1);
                default:
                        return -1;
        }
}
