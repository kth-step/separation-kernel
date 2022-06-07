// See LICENSE file for copyright and license details.
static uint64_t ms_slice(const CapMemorySlice ms, Cap *parent, Cap *child,
                  uint64_t begin, uint64_t end, uint64_t rwx) {
        /* Check memory slice validity */
        if (!(begin < end))
                return -1;
        /* Check bounds */
        if (!(ms.begin <= begin && end <= ms.end && (rwx & ms.rwx) == rwx))
                return -1;
        /* Check if has child */
        if (cap_is_child_ms(ms, cap_get(parent->next)))
                return -1;

        CapMemorySlice ms_child = cap_mk_memory_slice(begin, end, ms.rwx);
        return cap_set(child, cap_serialize_memory_slice(ms_child)) &&
               CapAppend(child, parent);
}

static uint64_t ms_split(const CapMemorySlice ms, Cap *parent, Cap *child0,
                  Cap *child1, uint64_t mid) {
        if (child0 == child1)
                return -1;
        /* Check if has child */
        if (cap_is_child_ms(ms, cap_get(parent->next)))
                return -1;
        /* Check bounds */
        if (!(ms.begin < mid && mid < ms.end))
                return -1;

        CapMemorySlice ms_child0 = cap_mk_memory_slice(ms.begin, mid, ms.rwx);
        CapMemorySlice ms_child1 = cap_mk_memory_slice(mid, ms.end, ms.rwx);
        return cap_set(child0, cap_serialize_memory_slice(ms_child0)) &&
               cap_set(child1, cap_serialize_memory_slice(ms_child1)) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

static uint64_t ms_instanciate(const CapMemorySlice ms, Cap *parent, Cap *child,
                        uint64_t addr, uint64_t rwx) {
        /* Check if it has a memory slice as child */
        CapData next = cap_get(parent->next);
        if (cap_is_child_ms(ms, next) && cap_get_type(next) == CAP_MEMORY_SLICE)
                return -1;

        /* Check bounds */
        uint64_t begin, end;
        pmp_napot_bounds(addr, &begin, &end);
        if (!(ms.begin <= begin && end <= ms.end && begin < end &&
              (rwx & ms.rwx) == rwx))
                return -1;

        CapPmpEntry pe_child = cap_mk_pmp_entry(addr, rwx);
        return cap_set(child, cap_serialize_pmp_entry(pe_child)) &&
               CapAppend(child, parent);
}

/*** MEMORY SLICE HANDLE ***/
uint64_t SyscallMemorySlice(const CapMemorySlice ms, Cap *cap, uint64_t a1,
                            uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                            uint64_t a6, uint64_t sysnr) {
        switch (sysnr) {
                case SYSNR_READ_CAP:
                        return cap_get_arr(cap, &current->args[1]);
                case SYSNR_MOVE_CAP:
                        return CapMove(curr_get_cap(a1), cap);
                case SYSNR_DELETE_CAP:
                        return CapDelete(cap);
                case SYSNR_REVOKE_CAP:
                        return CapRevoke(cap);
                case SYSNR_MS_SLICE:
                        return ms_slice(ms, cap, curr_get_cap(a1), a2, a3, a4);
                case SYSNR_MS_SPLIT:
                        return ms_split(ms, cap, curr_get_cap(a1),
                                        curr_get_cap(a2), a3);
                case SYSNR_MS_INSTANCIATE:
                        return ms_instanciate(ms, cap, curr_get_cap(a1), a2,
                                              a3);
                default:
                        return -1;
        }
}
