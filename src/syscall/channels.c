// See LICENSE file for copyright and license details.
#include "syscall.h"

static uint64_t ch_slice(const Cap cap, CapNode *cn, CapNode *child,
                         uint64_t begin, uint64_t end) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_CHANNELS);
        if (cap_is_child_ch(cap, cn_get(cn->next)))
                return -1;
        if (begin > end)
                return -1;
        Cap ch_child = cap_channels(begin, end);
        if (cap_is_child_ch_ch(cap, ch_child))
                return -1;
        return CapInsert(ch_child, child, cn);
}

static uint64_t ch_split(const Cap cap, CapNode *cn, CapNode *child0,
                         CapNode *child1, uint64_t mid) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_CHANNELS);
        if (cap_is_child_ch(cap, cn_get(cn->next)))
                return -1;
        if (child0 == child1)
                return -1;
        uint64_t begin = cap_channels_begin(cap);
        uint64_t end = cap_channels_end(cap);
        if (begin >= mid || mid <= end)
                return -1;
        Cap ch_child0 = cap_channels(begin, mid);
        Cap ch_child1 = cap_channels(mid, end);
        ASSERT(cap_is_child_ch_ch(cap, ch_child0));
        ASSERT(cap_is_child_ch_ch(cap, ch_child1));
        return CapInsert(ch_child0, child0, cn) &&
               CapInsert(ch_child1, child1, cn);
}

uint64_t SyscallChannels(const Cap cap, CapNode *cn, uint64_t a1, uint64_t a2,
                         uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6,
                         uint64_t a7) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_CHANNELS);
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
                        /* Delete channels */
                        return CapDelete(cn);
                case SYSNR_REVOKE_CAP:
                        /* Revoke all channels  */
                        return CapRevoke(cn);
                case SYSNR_CH_SLICE:
                        /* Slice channels */
                        return ch_slice(cap, cn, curr_get_cn(a1), a2, a3);
                case SYSNR_CH_SPLIT:
                        /* Split channels */
                        return ch_split(cap, cn, curr_get_cn(a1),
                                        curr_get_cn(a2), a3);
                default:
                        return -1;
        }
}
