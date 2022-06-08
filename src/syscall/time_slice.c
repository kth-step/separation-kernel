// See LICENSE file for copyright and license details.
#include "syscall.h"

static uint64_t ts_slice(const Cap cap, CapNode *cn, CapNode *child,
                        uint64_t begin, uint64_t end, uint64_t id,
                        uint64_t fuel) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_TIME_SLICE);

        if (cap_is_child_ts(cap, cn_get(cn->next)))
                return -1;

        /* Check validity of time slice */
        if (begin >= end || id > fuel)
                return -1;

        Cap cap_child =
            cap_time_slice(cap_time_slice_core(cap), begin, end, id, fuel);
        if (!cap_is_child_ts_ts(cap, cap_child))
                return -1;
        /* TODO: update schedule */
        return CapInsert(cap_child, child, cn);
}

static uint64_t ts_split(const Cap cap, CapNode *cn, CapNode *child0,
                         CapNode *child1, uint64_t quantum_mid, uint64_t fuel_mid) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_TIME_SLICE);
        if (child0 == child1)
                return -1;
        if (cap_is_child_ts(cap, cn_get(cn->next)))
                return -1;

        /* TODO: how to divide quantum and time slice */
        /* Maybe rename id to something else ... */
        uint64_t id_child0 = cap_time_slice_id(cap) + 1;
        uint64_t id_child1 = fuel_mid;
        uint64_t id = cap_time_slice_id(cap);
        uint64_t fuel = cap_time_slice_fuel(cap);
        uint64_t core = cap_time_slice_core(cap);
        uint64_t begin = cap_time_slice_begin(cap);
        uint64_t end = cap_time_slice_end(cap);
        if (begin >= (quantum_mid-1) || end <= quantum_mid || id_child0 > (fuel_mid-1) ||
            id_child1 > fuel)
                return -1;
        Cap cap_child0 =
            cap_time_slice(core, begin, quantum_mid-1, id_child0, fuel_mid-1);
        Cap cap_child1 =
            cap_time_slice(core, quantum_mid, end, id_child1, fuel);
        ASSERT(cap_is_child_ts_ts(cap, cap_child0));
        ASSERT(cap_is_child_ts_ts(cap, cap_child1));
        /* TODO: update schedule, must be after append */
        return CapInsert(cap_child0, child0, cn) &&
               CapInsert(cap_child1, child1, child0);
}

uint64_t SyscallTimeSlice(const Cap cap, CapNode *cn, uint64_t a1, uint64_t a2,
                          uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6,
                          uint64_t sysnr) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_TIME_SLICE);

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
                        /* TODO: update schedule */
                        return CapDelete(cn);
                case SYSNR_REVOKE_CAP:
                        /* Revoke time slice */
                        /* TODO: update schedule */
                        return CapRevoke(cn);
                case SYSNR_TS_SLICE:
                        /* Slice time */
                        return ts_slice(cap, cn, curr_get_cn(a1), a2, a3, a4,
                                        a5);
                case SYSNR_TS_SPLIT:
                        /* Split time */
                        return ts_split(cap, cn, curr_get_cn(a1),
                                        curr_get_cn(a2), a3, a4);
                default:
                        return -1;
        }
}

