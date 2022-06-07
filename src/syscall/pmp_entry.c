// See LICENSE file for copyright and license details.
uint64_t SyscallPmpEntry(const CapPmpEntry pe, Cap *cap, uint64_t a1,
                         uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                         uint64_t a6, uint64_t a7) {
        switch (a7) {
                case SYSNR_READ_CAP:
                        /* Read cap */
                        return cap_get_arr(cap, &current->args[1]);
                case SYSNR_MOVE_CAP:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case SYSNR_DELETE_CAP:
                        /* Delete cap */
                        /* Revoke unloads PMP by deleting a CapLoadedPmp */
                        CapRevoke(cap);
                        return CapDelete(cap);
                case SYSNR_PE_UNLOAD:
                        /* Unload cap */
                        return CapRevoke(cap);
                case SYSNR_PE_LOAD:
                        /* Load cap */
                        return ProcLoadPmp(current, pe, cap, a1);
                default:
                        return -1;
        }
}
