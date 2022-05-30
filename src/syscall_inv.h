// See LICENSE file for copyright and license details.
#pragma once

#include "cap.h"
#include "cap_util.h"
#include "proc.h"
#include "syscall.h"

static inline uint64_t SyscallMemorySlice(const CapMemorySlice ms, Cap *cap,
                                          uint64_t a1, uint64_t a2, uint64_t a3,
                                          uint64_t a4, uint64_t a5, uint64_t a6,
                                          uint64_t a7);
static inline uint64_t SyscallPmpEntry(const CapPmpEntry ms, Cap *cap,
                                       uint64_t a1, uint64_t a2, uint64_t a3,
                                       uint64_t a4, uint64_t a5, uint64_t a6,
                                       uint64_t a7);
static inline uint64_t SyscallTimeSlice(const CapTimeSlice ts, Cap *cap,
                                        uint64_t a1, uint64_t a2, uint64_t a3,
                                        uint64_t a4, uint64_t a5, uint64_t a6,
                                        uint64_t a7);
static inline uint64_t SyscallChannels(const CapChannels ch, Cap *cap,
                                       uint64_t a1, uint64_t a2, uint64_t a3,
                                       uint64_t a4, uint64_t a5, uint64_t a6,
                                       uint64_t a7);
static inline uint64_t SyscallReceiver(const CapReceiver receiver, Cap *cap,
                                       uint64_t a1, uint64_t a2, uint64_t a3,
                                       uint64_t a4, uint64_t a5, uint64_t a6,
                                       uint64_t a7);
static inline uint64_t SyscallSender(const CapSender sender, Cap *cap,
                                     uint64_t a1, uint64_t a2, uint64_t a3,
                                     uint64_t a4, uint64_t a5, uint64_t a6,
                                     uint64_t a7);
static inline uint64_t SyscallSupervisor(const CapSupervisor sup, Cap *cap,
                                         uint64_t a1, uint64_t a2, uint64_t a3,
                                         uint64_t a4, uint64_t a5, uint64_t a6,
                                         uint64_t a7);

/*** MEMORY SLICE HANDLE ***/
static inline uint64_t ms_slice(const CapMemorySlice ms, Cap *parent,
                                Cap *child, uint64_t begin, uint64_t end,
                                uint64_t rwx) {
        /* Check if has child */
        if (cap_is_child_ms(ms, cap_get(parent->next)))
                return -1;

        /* Check bounds */
        if (ms.begin > begin || end > ms.end || (rwx & ms.rwx) != rwx)
                return -1;

        CapMemorySlice ms_child = cap_mk_memory_slice(begin, end, ms.rwx);
        return cap_set_memory_slice(child, ms_child) &&
               CapAppend(child, parent);
}

static inline uint64_t ms_split(const CapMemorySlice ms, Cap *parent,
                                Cap *child0, Cap *child1, uint64_t mid) {
        if (child0 == child1)
                return -1;
        /* Check if has child */
        if (cap_is_child_ms(ms, cap_get(parent->next)))
                return -1;
        /* Check bounds */
        if (ms.begin >= mid || mid >= ms.end)
                return -1;

        CapMemorySlice ms_child0 = cap_mk_memory_slice(ms.begin, mid, ms.rwx);
        CapMemorySlice ms_child1 = cap_mk_memory_slice(mid, ms.end, ms.rwx);
        return cap_set_memory_slice(child0, ms_child0) &&
               cap_set_memory_slice(child1, ms_child1) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

static inline uint64_t ms_instanciate(const CapMemorySlice ms, Cap *parent,
                                      Cap *child, uint64_t addr, uint64_t rwx) {
        /* Check if it has a memory slice as child */
        CapUnion next = cap_get(parent->next);
        if (cap_is_child_ms(ms, next) && next.type == CAP_MEMORY_SLICE)
                return -1;

        /* Check bounds */
        uint64_t begin, end;
        pmp_napot_bounds(addr, &begin, &end);
        if (ms.begin > begin || end > ms.end || begin >= end ||
            (rwx & ms.rwx) != rwx)
                return -1;

        CapPmpEntry pe_child = cap_mk_pmp_entry(addr, rwx);
        return cap_set_pmp_entry(child, pe_child) && CapAppend(child, parent);
}

uint64_t SyscallMemorySlice(const CapMemorySlice ms, Cap *cap, uint64_t a1,
                            uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                            uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        return cap_get_data(cap, &current->args[1]);
                case 1:
                        return CapMove(curr_get_cap(a1), cap);
                case 2:
                        return CapDelete(cap);
                case 3:
                        return CapRevoke(cap);
                case 4:
                        return ms_slice(ms, cap, curr_get_cap(a1), a2, a3, a4);
                case 5:
                        return ms_split(ms, cap, curr_get_cap(a1),
                                        curr_get_cap(a2), a3);
                case 6:
                        return ms_instanciate(ms, cap, curr_get_cap(a1), a2,
                                              a3);
                default:
                        return -1;
        }
}

static inline uint64_t pe_load(const CapPmpEntry pe, Cap *parent,
                               uint8_t index) {
        if (index > 8)
                return -1;
        Cap *child = &current->pmp_table[index];
        CapLoadedPmp lp_child = cap_mk_loaded_pmp(pe.addr, pe.rwx);
        if (!cap_set_loaded_pmp(child, lp_child))
                return -1;
        if (!CapAppend(child, parent)) {
                child->data[1] = 0;
                return 0;
        }
        return 1;
}

/*** PMP ENTRY HANDLE ***/
uint64_t SyscallPmpEntry(const CapPmpEntry pe, Cap *cap, uint64_t a1,
                         uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                         uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        /* Read cap */
                        return cap_get_data(cap, &current->args[1]);
                case 1:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case 2:
                        /* Delete cap */
                        /* Revoke unloads PMP by deleting a CapLoadedPmp */
                        CapRevoke(cap);
                        return CapDelete(cap);
                case 3:
                        /* Unload cap */
                        return CapRevoke(cap);
                case 4:
                        /* Load cap */
                        return pe_load(pe, cap, a1);
                default:
                        return -1;
        }
}

/*** TIME SLICE HANDLE ***/
static inline uint64_t ts_slice(const CapTimeSlice ts, Cap *parent, Cap *child,
                                uint64_t begin, uint64_t end, uint64_t tsid,
                                uint64_t tsid_end) {
        CapUnion next = cap_get(parent->next);
        if (cap_is_child_ts(ts, next))
                return -1;
        if (ts.begin > begin || end > ts.end || begin >= end ||
            ts.tsid >= tsid || ts.tsid_end > tsid_end || tsid > tsid_end)
                return -1;
        CapTimeSlice ts_child =
            cap_mk_time_slice(ts.hartid, begin, end, tsid, tsid_end);
        /* TODO: update schedule */
        return cap_set_time_slice(child, ts_child) && CapAppend(child, parent);
}

static inline uint64_t ts_split(const CapTimeSlice ts, Cap *parent, Cap *child0,
                                Cap *child1, uint64_t qmid, uint64_t tsmid) {
        CapUnion next = cap_get(parent->next);
        if (cap_is_child_ts(ts, next) || child0 == child1)
                return -1;
        if (ts.begin >= qmid || (qmid + 1) >= ts.end || ts.tsid >= tsmid ||
            (tsmid + 1) > ts.tsid_end)
                return -1;
        CapTimeSlice ts_child0 =
            cap_mk_time_slice(ts.hartid, ts.begin, qmid, ts.tsid + 1, tsmid);
        CapTimeSlice ts_child1 = cap_mk_time_slice(ts.hartid, qmid + 1, ts.end,
                                                   tsmid + 1, ts.tsid_end);
        /* TODO: update schedule, must be after append */
        return cap_set_time_slice(child0, ts_child0) &&
               cap_set_time_slice(child1, ts_child1) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

uint64_t SyscallTimeSlice(const CapTimeSlice ts, Cap *cap, uint64_t a1,
                          uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                          uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        /* Read cap */
                        return cap_get_data(cap, &current->args[1]);
                case 1:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case 2:
                        /* Delete time slice */
                        /* TODO: update schedule */
                        return CapDelete(cap);
                case 3:
                        /* Revoke time slice */
                        /* TODO: update schedule */
                        return CapRevoke(cap);
                case 4:
                        /* Slice time */
                        return ts_slice(ts, cap, curr_get_cap(a1), a2, a3, a4,
                                        a5);
                case 5:
                        /* Split time */
                        return ts_split(ts, cap, curr_get_cap(a1),
                                        curr_get_cap(a2), a3, a4);
                default:
                        return -1;
        }
}

/*** CHANNELS HANDLE ***/
uint64_t ch_slice(const CapChannels ch, Cap *parent, Cap *child, uint64_t begin,
                  uint64_t end) {
        CapUnion next = cap_get(parent->next);
        if (cap_is_child_ch(ch, next))
                return -1;
        if (begin > end)
                return -1;
        CapChannels ch_child = cap_mk_channels(begin, end);
        if (cap_is_child_ch_ch(ch, ch_child))
                return -1;
        return cap_set_channels(child, ch_child) && CapAppend(child, parent);
}

uint64_t ch_split(const CapChannels ch, Cap *parent, Cap *child0, Cap *child1,
                  uint64_t mid) {
        if (cap_is_child_ch(ch, cap_get(parent->next)))
                return -1;
        if (child0 == child1)
                return -1;
        if (!(ch.begin < mid && mid <= ch.end))
                return -1;
        CapChannels ch_child0 = cap_mk_channels(ch.begin, mid);
        CapChannels ch_child1 = cap_mk_channels(mid, ch.end);
        return cap_set_channels(child0, ch_child0) &&
               cap_set_channels(child1, ch_child1) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

uint64_t ch_instanciate(const CapChannels ch, Cap *parent, Cap *child0,
                        Cap *child1, uint64_t channel) {
        /* TODO: Check if channel is used */
        if (child0 == child1)
                return -1;
        CapReceiver recv_child0 = cap_mk_receiver(channel);
        CapSender send_child1 = cap_mk_sender(channel);
        if (!(ch.begin <= channel && channel <= ch.end))
                return -1;
        return cap_set_receiver(child0, recv_child0) &&
               cap_set_sender(child1, send_child1) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

uint64_t SyscallChannels(const CapChannels ch, Cap *cap, uint64_t a1,
                         uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                         uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        /* Read cap */
                        return cap_get_data(cap, &current->args[1]);
                case 1:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case 2:
                        /* Delete channels */
                        return CapDelete(cap);
                case 3:
                        /* Revoke all channels  */
                        return CapRevoke(cap);
                case 4:
                        /* Slice channels */
                        return ch_slice(ch, cap, curr_get_cap(a1), a2, a3);
                case 5:
                        /* Split channels */
                        return ch_split(ch, cap, curr_get_cap(a1),
                                        curr_get_cap(a2), a3);
                case 6:
                        /* Instanciate channels, make receiver and sender */
                        return ch_instanciate(ch, cap, curr_get_cap(a1),
                                              curr_get_cap(a2), a3);
                default:
                        return -1;
        }
}

/*** RECEIVER HANDLER ***/
uint64_t SyscallReceiver(const CapReceiver receiver, Cap *cap, uint64_t a1,
                         uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                         uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        /* Read cap */
                        return cap_get_data(cap, &current->args[1]);
                case 1:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case 2:
                        /* Delete time slice */
                        return CapDelete(cap);
                case 3:
                        /* Delete time slice */
                        return CapRevoke(cap);
                case 4:
                        /* Receive message */
                default:
                        return -1;
        }
}

/*** SENDER HANDLER ***/
uint64_t SyscallSender(const CapSender sender, Cap *cap, uint64_t a1,
                       uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                       uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        /* Read cap */
                        return cap_get_data(cap, &current->args[1]);
                case 1:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case 2:
                        /* Delete time slice */
                        return CapDelete(cap);
                case 3:
                        /* Delete time slice */
                        return CapRevoke(cap);
                case 4:
                        /* Send message */
                default:
                        return -1;
        }
}

/*** SUPERVISOR HANDLER ***/
uint64_t SyscallSupervisor(const CapSupervisor sup, Cap *cap, uint64_t a1,
                           uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                           uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        /* Read cap */
                        return cap_get_data(cap, &current->args[1]);
                case 1:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case 2:
                        /* Delete time slice */
                        return CapDelete(cap);
                case 3:
                        /* Delete time slice */
                        return CapRevoke(cap);
                case 4:
                        /* Halt process */
                case 5:
                        /* Resume process */
                case 6:
                        /* Reset process */
                case 7:
                        /* Read cap */
                case 8:
                        /* Give cap */
                case 9:
                        /* Take cap */
                default:
                        return -1;
        }
}
