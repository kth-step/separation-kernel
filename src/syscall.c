// See LICENSE file for copyright and license details.

#include <assert.h>
#include <stdint.h>

#include "cap.h"
#include "cap_util.h"
#include "proc.h"
#include "stack.h"

/* Read process ID. */
void SyscallReadPid(void);

/* Read cap at cs. */
void SyscallCapRead(uint64_t cs);
/* Move cap from cs to cd. */
void SyscallCapMove(uint64_t cd, uint64_t cs);
/* Revoke cap at cs */
void SyscallCapRevoke(uint64_t cs);
/* Delete cap at cs */
void SyscallCapDelete(uint64_t cs);

/* Make memory slice at cd using memory slice at cs. */
void SyscallMemorySlice(uint64_t cd, uint64_t cs, uint64_t begin, uint64_t end,
                        uint64_t rwx);

/* Split memory slice at cs, put them in cd0 and cd1. */
void SyscallMemorySplit(uint64_t cd0, uint64_t cd1, uint64_t cs, uint64_t mid);

Cap *curr_get_cap(uint64_t idx) {
        return &current->cap_table[idx % N_CAPS];
}

/* Get process ID */
void SyscallReadPid(void) {
        current->args[0] = current->pid;
}

/* Read a capability. */
void SyscallCapRead(uintptr_t cs) {
        Cap *cap = curr_get_cap(cs);
        if (cap_get_data(cap, &current->args[1])) {
                current->args[0] = 1;
        } else {
                current->args[0] = 0;
                current->args[1] = 0;
                current->args[2] = 0;
        }
}

void SyscallCapMove(uintptr_t cd, uintptr_t cs) {
        Cap *src = curr_get_cap(cs);
        Cap *dest = curr_get_cap(cd);
        current->args[0] = CapMove(dest, src);
}

void SyscallCapDelete(uintptr_t cs) {
        current->args[0] = CapDelete(curr_get_cap(cs));
}

void SyscallCapRevoke(uintptr_t cs) {
        CapRevoke(curr_get_cap(cs));
}

void SyscallMemorySlice(uint64_t cd, uint64_t cs, uint64_t begin, uint64_t end,
                        uint64_t rwx) {
        Cap *src = curr_get_cap(cs);
        Cap *dest = curr_get_cap(cd);
        CapMemorySlice ms_src = cap_get_memory_slice(src);
        CapMemorySlice ms_dest = cap_mk_memory_slice(begin, end, rwx);
        if (!ms_src.valid || !cap_is_deleted(dest) || !ms_dest.valid ||
            !cap_is_child_ms_ms(ms_src, ms_dest)) {
                current->args[0] = -1;
        } else {
                cap_set_memory_slice(dest, ms_dest);
                current->args[0] = CapAppend(dest, src);
        }
}

void SyscallMemorySplit(uint64_t cd0, uint64_t cd1, uint64_t cs, uint64_t mid) {
        Cap *src = curr_get_cap(cs);
        Cap *dest0 = curr_get_cap(cd0);
        Cap *dest1 = curr_get_cap(cd1);

        CapMemorySlice ms_src = cap_get_memory_slice(src);
        CapMemorySlice ms_dest0 =
            cap_mk_memory_slice(ms_src.begin, mid, ms_src.rwx);
        CapMemorySlice ms_dest1 =
            cap_mk_memory_slice(mid, ms_src.end, ms_src.rwx);

        if (!ms_src.valid || !cap_is_deleted(dest0) || !cap_is_deleted(dest1) ||
            !ms_dest0.valid || !ms_dest1.valid) {
                current->args[0] = -1;
        } else {
                cap_set_memory_slice(dest0, ms_dest0);
                cap_set_memory_slice(dest1, ms_dest1);
                current->args[0] =
                    CapAppend(dest0, src) && CapAppend(dest1, src);
        }
}

void SyscallTimeSlice(uint64_t cd, uint64_t cs, uint64_t begin, uint64_t end,
                      uint64_t tsid, uint64_t tsid_end) {
        Cap *src = curr_get_cap(cs);
        Cap *dest = curr_get_cap(cd);
        CapTimeSlice ts_src = cap_get_time_slice(src);
        CapTimeSlice ts_dest =
            cap_mk_time_slice(ts_src.hartid, begin, end, tsid, tsid_end);

        if (!ts_src.valid || !cap_is_deleted(dest) || !ts_dest.valid ||
            !cap_is_child_ts_ts(ts_src, ts_dest)) {
                current->args[0] = -1;
        } else {
                cap_set_time_slice(dest, ts_dest);
                current->args[0] = CapAppend(dest, src);
        }
}

void SyscallTimeSplit(uint64_t cd0, uint64_t cd1, uint64_t cs,
                      uint64_t mid_quantum, uint64_t mid_id) {
        Cap *src = curr_get_cap(cs);
        Cap *dest0 = curr_get_cap(cd0);
        Cap *dest1 = curr_get_cap(cd1);

        CapTimeSlice ts_src = cap_get_time_slice(src);
        CapTimeSlice ts_dest0 = cap_mk_time_slice(
            ts_src.hartid, ts_src.begin, mid_quantum, ts_src.tsid + 1, mid_id);
        CapTimeSlice ts_dest1 =
            cap_mk_time_slice(ts_src.hartid, mid_quantum, ts_src.end, mid_id,
                              ts_src.tsid_end);

        if (!ts_src.valid || !cap_is_deleted(dest0) || !cap_is_deleted(dest1) ||
            !ts_dest0.valid || !ts_dest1.valid) {
                current->args[0] = -1;
        } else {
                cap_set_time_slice(dest0, ts_dest0);
                cap_set_time_slice(dest1, ts_dest1);
                current->args[0] =
                    CapAppend(dest0, src) && CapAppend(dest1, src);
        }
}
