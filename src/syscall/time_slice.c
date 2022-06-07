// See LICENSE file for copyright and license details.
static int64_t ts_slice(const CapTimeSlice ts, Cap *parent, Cap *child,
                  uint64_t begin, uint64_t end, uint64_t tsid,
                  uint64_t tsid_end) {
        /* Check validity of time slice */
        if (!(begin < end && tsid < tsid_end))
                return -1;
        /* Check containment */
        if (!(ts.begin <= begin && end <= ts.end && ts.tsid < tsid &&
              tsid_end <= ts.tsid_end))
                return -1;
        if (cap_is_child_ts(ts, cap_get(parent->next)))
                return -1;
        CapTimeSlice ts_child =
            cap_mk_time_slice(ts.hartid, begin, end, tsid, tsid_end);
        /* TODO: update schedule */
        return cap_set(child, cap_serialize_time_slice(ts_child)) &&
               CapAppend(child, parent);
}

static uint64_t ts_split(const CapTimeSlice ts, Cap *parent, Cap *child0, Cap *child1,
                  uint64_t qmid, uint64_t tsmid) {
        /* Check that qmid and tsmid are in middle of parents time slice and
         * tsid intervals */
        if (!(ts.begin < qmid && qmid < ts.end && (ts.tsid + 1) < tsmid &&
              tsmid < ts.tsid_end))
                return -1;
        if (cap_is_child_ts(ts, cap_get(parent->next)) || child0 == child1)
                return -1;
        CapTimeSlice ts_child0 =
            cap_mk_time_slice(ts.hartid, ts.begin, qmid, ts.tsid + 1, tsmid);
        CapTimeSlice ts_child1 = cap_mk_time_slice(ts.hartid, qmid + 1, ts.end,
                                                   tsmid + 1, ts.tsid_end);
        /* TODO: update schedule, must be after append */
        return cap_set(child0, cap_serialize_time_slice(ts_child0)) &&
               cap_set(child1, cap_serialize_time_slice(ts_child1)) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

uint64_t SyscallTimeSlice(const CapTimeSlice ts, Cap *cap, uint64_t a1,
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
                        /* Delete time slice */
                        /* TODO: update schedule */
                        return CapDelete(cap);
                case SYSNR_REVOKE_CAP:
                        /* Revoke time slice */
                        /* TODO: update schedule */
                        return CapRevoke(cap);
                case SYSNR_TS_SLICE:
                        /* Slice time */
                        return ts_slice(ts, cap, curr_get_cap(a1), a2, a3, a4,
                                        a5);
                case SYSNR_TS_SPLIT:
                        /* Split time */
                        return ts_split(ts, cap, curr_get_cap(a1),
                                        curr_get_cap(a2), a3, a4);
                default:
                        return -1;
        }
}

