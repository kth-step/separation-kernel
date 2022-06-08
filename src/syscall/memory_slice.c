// See LICENSE file for copyright and license details.
#include "syscall.h"
static uint64_t ms_slice(const Cap cap, CapNode *cn, CapNode *child,
                         uint64_t begin, uint64_t end, uint64_t rwx) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_MEMORY_SLICE);
        /* Check if has child */
        if (cap_is_child_ms(cap, cn_get(cn->next)))
                return -1;
        /* Check memory slice validity */
        if (!(begin < end))
                return -1;
        /* Check bounds */
        Cap cap_child = cap_memory_slice(begin, end, rwx);
        if (!cap_is_child_ms_ms(cap, cap_child))
                return -1;

        return CapInsert(cap_child, child, cn);
}

static uint64_t ms_split(const Cap cap, CapNode *cn, CapNode *child0,
                         CapNode *child1, uint64_t mid) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_MEMORY_SLICE);
        if (child0 == child1)
                return -1;
        /* Check if has child */
        if (cap_is_child_ms(cap, cn_get(cn->next)))
                return -1;
        /* Check bounds */
        uint64_t begin = cap_memory_slice_begin(cap);
        uint64_t end = cap_memory_slice_end(cap);
        uint64_t rwx = cap_memory_slice_rwx(cap);
        if (!(begin < mid && mid < end))
                return -1;
        Cap cap_child0 = cap_memory_slice(begin, mid, rwx);
        Cap cap_child1 = cap_memory_slice(mid, end, rwx);
        ASSERT(cap_is_child_ms_ms(cap, cap_child0));
        ASSERT(cap_is_child_ms_ms(cap, cap_child1));
        return CapInsert(cap_child0, child0, cn) &&
               CapInsert(cap_child1, child1, cn);
}

static uint64_t ms_instanciate(const Cap cap, CapNode *cn, CapNode *child,
                               uint64_t pmpaddr, uint64_t pmprwx) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_MEMORY_SLICE);
        /* Check if it has a memory slice as child */

        Cap next = cn_get(cn->next);
        if (cap_is_child_ms(cap, next) &&
            cap_get_type(next) == CAP_TYPE_MEMORY_SLICE)
                return -1;

        Cap pe_child = cap_pmp_entry(pmpaddr, pmprwx);
        if (!cap_is_child_ms_pe(cap, pe_child))
                return -1;

        return CapInsert(pe_child, child, cn);
}

/*** MEMORY SLICE HANDLE ***/
uint64_t SyscallMemorySlice(const Cap cap, CapNode *cn, uint64_t a1,
                            uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                            uint64_t a6, uint64_t sysnr) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_MEMORY_SLICE);
        switch (sysnr) {
                case SYSNR_READ_CAP:
                        current->args[1] = cap.word0;
                        current->args[2] = cap.word1;
                        return 1;
                case SYSNR_MOVE_CAP:
                        return CapMove(curr_get_cn(a1), cn);
                case SYSNR_DELETE_CAP:
                        return CapDelete(cn);
                case SYSNR_REVOKE_CAP:
                        return CapRevoke(cn);
                case SYSNR_MS_SLICE:
                        return ms_slice(cap, cn, curr_get_cn(a1), a2, a3, a4);
                case SYSNR_MS_SPLIT:
                        return ms_split(cap, cn, curr_get_cn(a1),
                                        curr_get_cn(a2), a3);
                case SYSNR_MS_INSTANCIATE:
                        return ms_instanciate(cap, cn, curr_get_cn(a1), a2, a3);
                default:
                        return -1;
        }
}
