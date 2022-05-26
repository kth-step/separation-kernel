// See LICENSE file for copyright and license details.

#include "syscall.h"

#include <assert.h>
#include <stdint.h>

#include "cap.h"
#include "cap_util.h"
#include "proc.h"
#include "sched.h"
#include "stack.h"

/* Read process ID. */
void SyscallReadPid(void);

/* Get process ID */
void SyscallReadPid(void) {
        current->args[0] = current->pid;
}

/* Read a capability. */
void SyscallReadCap(uintptr_t cs) {
        Cap *cap = curr_get_cap(cs);
        if (cap_get_data(cap, &current->args[1])) {
                current->args[0] = 1;
        } else {
                current->args[0] = 0;
                current->args[1] = 0;
                current->args[2] = 0;
        }
}

void SyscallMoveCap(uintptr_t cd, uintptr_t cs) {
        Cap *src = curr_get_cap(cs);
        Cap *dest = curr_get_cap(cd);
        current->args[0] = CapMove(dest, src);
}

void SyscallDeleteCap(uintptr_t cs) {
        current->args[0] = CapDelete(curr_get_cap(cs));
}

void SyscallRevokeCap(uintptr_t cs) {
        CapRevoke(curr_get_cap(cs));
}

void SyscallSliceMemory(uint64_t cd, uint64_t cs, uint64_t begin, uint64_t end,
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

void SyscallSplitMemory(uint64_t cd0, uint64_t cd1, uint64_t cs, uint64_t mid) {
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

void SyscallMakePmp(uint64_t cid_pmp, uint64_t cid_ms, uint64_t addr,
                    uint64_t rwx) {
        Cap *cap_pmp = curr_get_cap(cid_pmp);
        Cap *cap_ms = curr_get_cap(cid_ms);
        CapMemorySlice ms = cap_get_memory_slice(cap_ms);
        CapPmpEntry pmp = cap_mk_pmp_entry(addr, rwx & 0x7);
        if (!ms.valid || !cap_is_deleted(cap_pmp) || !pmp.valid) {
                current->args[0] = -1;
                return;
        }
        cap_set_pmp_entry(cap_pmp, pmp);
        current->args[0] = CapAppend(cap_pmp, cap_ms);
}

void SyscallLoadPmp(uint64_t cid_pmp, uint64_t index) {
        Cap *cap_pmp = curr_get_cap(cid_pmp);
        CapPmpEntry pmp = cap_get_pmp_entry(cap_pmp);
        if (!pmp.valid) {
                current->args[0] = -1;
                return;
        }
        /* TODO: set index in pmp */
        current->args[0] = ProcPmpLoad(current->pid, index, pmp.addr, pmp.rwx);
        /* If cap_pmp is deleted, unload pmp */
        if (cap_is_deleted(cap_pmp) && current->args[0])
                ProcPmpUnload(current->pid, index);
}
void SyscallUnloadPmp(uint64_t cid_pmp) {
        Cap *cap_pmp = curr_get_cap(cid_pmp);
        CapPmpEntry pmp = cap_get_pmp_entry(cap_pmp);
        if (!pmp.valid) {
                current->args[0] = -1;
                return;
        }
        current->args[0] = ProcPmpUnload(current->pid, pmp.index);
}

void SyscallSliceTime(uint64_t cd, uint64_t cs, uint64_t begin, uint64_t end,
                      uint64_t tsid, uint64_t tsid_end) {
        Cap *src = curr_get_cap(cs);
        Cap *dest = curr_get_cap(cd);
        CapTimeSlice ts_src = cap_get_time_slice(src);
        CapTimeSlice ts_dest =
            cap_mk_time_slice(ts_src.hartid, begin, end, tsid, tsid_end);

        if (!ts_src.valid || !cap_is_deleted(dest) || !ts_dest.valid ||
            !cap_is_child_ts_ts(ts_src, ts_dest)) {
                current->args[0] = -1;
                return;
        }
        uint8_t pid = current->pid;
        SchedUpdatePidTsid(ts_dest.begin, ts_dest.end, ts_dest.hartid, pid,
                           ts_src.tsid, pid, ts_dest.tsid);
        cap_set_time_slice(dest, ts_dest);
        current->args[0] = CapAppend(dest, src);
}

void SyscallSplitTime(uint64_t cd0, uint64_t cd1, uint64_t cs,
                      uint64_t mid_quantum, uint64_t mid_id) {
        Cap *src = curr_get_cap(cs);
        Cap *dest0 = curr_get_cap(cd0);
        Cap *dest1 = curr_get_cap(cd1);

        CapTimeSlice ts_src = cap_get_time_slice(src);
        CapTimeSlice ts_dest0 = cap_mk_time_slice(
            ts_src.hartid, ts_src.begin, mid_quantum, ts_src.tsid + 1, mid_id);
        CapTimeSlice ts_dest1 =
            cap_mk_time_slice(ts_src.hartid, mid_quantum + 1, ts_src.end,
                              mid_id + 1, ts_src.tsid_end);

        if (!ts_src.valid || !cap_is_deleted(dest0) || !cap_is_deleted(dest1) ||
            !ts_dest0.valid || !ts_dest1.valid || dest0 == dest1) {
                current->args[0] = -1;
                return;
        }
        uint8_t pid = current->pid;
        SchedUpdatePidTsid(ts_dest0.begin, ts_dest0.end, ts_dest0.hartid, pid,
                           ts_src.tsid, pid, ts_dest0.tsid);
        cap_set_time_slice(dest0, ts_dest0);
        SchedUpdatePidTsid(ts_dest1.begin, ts_dest1.end, ts_dest1.hartid, pid,
                           ts_src.tsid, pid, ts_dest1.tsid);
        cap_set_time_slice(dest1, ts_dest1);
        current->args[0] = CapAppend(dest0, src) && CapAppend(dest1, src);
}

void SyscallSliceChannels(uint64_t dest, uint64_t src, uint64_t begin,
                          uint64_t end) {
        Cap *cap_src = curr_get_cap(src);
        Cap *cap_dest = curr_get_cap(dest);

        CapChannels ch_old = cap_get_channels(cap_src);
        CapChannels ch_new = cap_mk_channels(begin, end);
        if (!ch_old.valid || !ch_new.valid || !cap_is_deleted(cap_dest)) {
                current->args[0] = -1;
                return;
        }
        cap_set_channels(cap_dest, ch_new);
        current->args[0] = CapAppend(cap_dest, cap_src);
}

void SyscallSplitChannels(uint64_t dest0, uint64_t dest1, uint64_t src,
                          uint64_t mid) {
        Cap *cap_src = curr_get_cap(src);
        Cap *cap_dest0 = curr_get_cap(dest0);
        Cap *cap_dest1 = curr_get_cap(dest1);

        CapChannels ch_old = cap_get_channels(cap_src);
        CapChannels ch_new0 = cap_mk_channels(ch_old.begin, mid);
        CapChannels ch_new1 = cap_mk_channels(mid, ch_old.end);
        if (!ch_old.valid || !ch_new0.valid || !ch_new1.valid ||
            !cap_is_deleted(cap_dest0) || !cap_is_deleted(cap_dest1)) {
                current->args[0] = -1;
                return;
        }
        cap_set_channels(cap_dest0, ch_new0);
        cap_set_channels(cap_dest1, ch_new1);
        current->args[0] =
            CapAppend(cap_dest0, cap_src) && CapAppend(cap_dest1, cap_src);
}

void SyscallMakeReceiver(uint64_t cid_recv, uint64_t cid_ch, uint64_t epid) {
        Cap *cap_ch = curr_get_cap(cid_ch);
        Cap *cap_recv = curr_get_cap(cid_recv);
        CapChannels ch = cap_get_channels(cap_ch);
        CapEndpoint ep_recv = cap_mk_endpoint(epid, true);
        if (!ch.valid || !ep_recv.valid || !cap_is_deleted(cap_recv) ||
            !cap_is_child_ch_ep(ch, ep_recv)) {
                current->args[0] = -1;
                return;
        }
        if (!cap_cas_recv(epid, -1, current->pid)) {
                current->args[0] = -1;
                return;
        }
        cap_set_endpoint(cap_recv, ep_recv);
        current->args[0] = CapAppend(cap_recv, cap_ch);
}

void SyscallMakeSender(uint64_t cid_send, uint64_t cid_recv) {
        Cap *cap_recv = curr_get_cap(cid_recv);
        Cap *cap_send = curr_get_cap(cid_send);
        CapEndpoint ep_recv = cap_get_endpoint(cap_recv);
        CapEndpoint ep_send = cap_mk_endpoint(ep_recv.epid, false);
        if (!ep_recv.valid || !ep_send.valid || !cap_is_deleted(cap_send) ||
            !ep_recv.is_receiver || !cap_is_child_ep_ep(ep_recv, ep_send)) {
                current->args[0] = -1;
                return;
        }
        cap_set_endpoint(cap_send, ep_send);
        current->args[0] = CapAppend(cap_send, cap_recv);
}

void SyscallReceive(uint64_t cid_recv, uint64_t caps_dest) {
        Cap *cap_recv = curr_get_cap(cid_recv);
        CapEndpoint ep_recv = cap_get_endpoint(cap_recv);
        if (!ep_recv.valid || !ep_recv.is_receiver) {
                current->args[0] = -1;
                return;
        }
        current->epid = ep_recv.epid;
        current->args[0] = 0;
        while (current->epid != -1) {
                __asm__ volatile("");
        }
}

void SyscallSend(uint64_t cid_send, uint64_t caps_src, uint64_t is_blocking,
                 uint64_t m0, uint64_t m1, uint64_t m2, uint64_t m3) {
        Cap *cap_send = curr_get_cap(cid_send);
        CapEndpoint ep_send = cap_get_endpoint(cap_send);
        int8_t pid = ep_send.epid;
        if (!ep_send.valid || ep_send.is_receiver || pid < 0) {
                current->args[0] = -1;
                return;
        }
        current->args[0] = 0;
        Proc *receiver = &processes[pid];
        do {
                if (receiver->epid == ep_send.epid) {
                        receiver->args[0] = 1;
                        receiver->args[2] = m0;
                        receiver->args[3] = m1;
                        receiver->args[4] = m2;
                        receiver->args[5] = m3;
                        /* Get capabilities to send */
                        uint64_t caps_dest = receiver->args[1];
                        for (int i = 0; i < 64; i += 8) {
                                uint8_t cid_src = caps_src >> i;
                                uint8_t cid_dest = caps_dest >> i;
                                if (cid_src == 0 || cid_dest == 0)
                                        break;
                                Cap *src = curr_get_cap(cid_src);
                                Cap *dest = proc_get_cap(receiver, cid_dest);
                                bool succ = CapInterprocessMove(
                                    dest, src, receiver->pid, current->pid);
                                if (!succ)
                                        break;
                        }
                        __sync_synchronize();
                        receiver->epid = -1;
                        current->args[0] = 1;
                }
        } while (is_blocking);
}

void SyscallHalt(uint64_t cid_sup) {
        Cap *cap_sup = curr_get_cap(cid_sup);
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        if (!sup.valid) {
                current->args[0] = -1;
                return;
        }
        Proc *proc = &processes[sup.pid];
        proc->halt = 1;
        current->args[0] = __sync_bool_compare_and_swap(
            &current->state, PROC_SUSPENDED, PROC_HALTED);
}

void SyscallResume(uint64_t cid_sup) {
        Cap *cap_sup = curr_get_cap(cid_sup);
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        if (!sup.valid) {
                current->args[0] = -1;
                return;
        }
        Proc *proc = &processes[sup.pid];
        proc->halt = 0;
        current->args[0] = __sync_bool_compare_and_swap(
            &proc->state, PROC_HALTED, PROC_SUSPENDED);
}

void SyscallGiveCap(uint64_t cid_sup, uint64_t cid_dest, uint64_t cid_src) {
        Cap *cap_sup = curr_get_cap(cid_sup);
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        if (!sup.valid) {
                current->args[0] = -1;
                return;
        }
        Proc *proc = &processes[sup.pid];
        if (proc->state != PROC_HALTED) {
                current->args[0] = -2;
                return;
        }
        Cap *cap_src = curr_get_cap(cid_src);
        Cap *cap_dest = proc_get_cap(proc, cid_dest);
        if (cap_is_deleted(cap_src) || !cap_is_deleted(cap_dest)) {
                current->args[0] = -3;
                return;
        }
        current->args[0] =
            CapInterprocessMove(cap_dest, cap_src, proc->pid, current->pid);
}

void SyscallTakeCap(uint64_t cid_sup, uint64_t cid_dest, uint64_t cid_src) {
        Cap *cap_sup = curr_get_cap(cid_sup);
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        if (!sup.valid) {
                current->args[0] = -1;
                return;
        }
        Proc *proc = &processes[sup.pid];
        if (proc->state != PROC_HALTED) {
                current->args[0] = -2;
                return;
        }
        Cap *cap_src = proc_get_cap(proc, cid_src);
        Cap *cap_dest = curr_get_cap(cid_dest);
        if (cap_is_deleted(cap_src) || !cap_is_deleted(cap_dest)) {
                current->args[0] = -3;
                return;
        }
        current->args[0] =
            CapInterprocessMove(cap_dest, cap_src, current->pid, proc->pid);
}

void SyscallSupReadCap(uint64_t cid_sup, uint64_t cid) {
        Cap *cap_sup = curr_get_cap(cid_sup);
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        if (!sup.valid) {
                current->args[0] = -1;
                return;
        }
        Proc *proc = &processes[sup.pid];
        Cap *cap = proc_get_cap(proc, cid);
        if (proc->state != PROC_HALTED) {
                current->args[0] = -1;
                return;
        }
        if (cap_get_data(cap, &current->args[1])) {
                current->args[0] = 1;
        } else {
                current->args[0] = 0;
                current->args[1] = 0;
                current->args[2] = 0;
        }
}

void SyscallReset(uint64_t cid_sup, uint64_t cid_pmp) {
        Cap *cap_sup = curr_get_cap(cid_sup);
        CapSupervisor sup = cap_get_supervisor(cap_sup);
        if (!sup.valid || processes[sup.pid].state != PROC_HALTED) {
                current->args[0] = -1;
                return;
        }
        Cap *cap_pmp = curr_get_cap(cid_pmp);
        CapPmpEntry pmp = cap_get_pmp_entry(cap_pmp);
        if (!pmp.valid) {
                current->args[0] = -1;
                return;
        }
        ProcPmpUnload(current->pid, pmp.index);
        current->args[0] = ProcReset(sup.pid, cap_pmp);
}
