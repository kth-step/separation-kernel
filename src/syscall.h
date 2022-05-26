// See LICENSE file for copyright and license details.

#include <stdint.h>

#include "cap.h"
#include "proc.h"

/* Read process ID. */
void SyscallReadPid(void);

/* Read cap at cs. */
void SyscallReadCap(uint64_t cs);
/* Move cap from cs to cd. */
void SyscallMoveCap(uint64_t cd, uint64_t cs);
/* Revoke cap at cs */
void SyscallRevokeCap(uint64_t cs);
/* Delete cap at cs */
void SyscallDeleteCap(uint64_t cs);

/* Make memory slice at cd using memory slice at cs. */
void SyscallSliceMemory(uint64_t cd, uint64_t cs, uint64_t begin, uint64_t end,
                        uint64_t rwx);
/* Split memory slice at cs, put them in cd0 and cd1. */
void SyscallSplitMemory(uint64_t cd0, uint64_t cd1, uint64_t cs, uint64_t mid);

/* PMP Entry */
void SyscallMakePmp(uint64_t cid_pmp, uint64_t cid_ms, uint64_t addr,
                    uint64_t rwx);
void SyscallLoadPmp(uint64_t cid_pmp, uint64_t index);
void SyscallUnloadPmp(uint64_t cid_pmp);

/* Time Slice */
void SyscallSliceTime(uint64_t cd, uint64_t cs, uint64_t begin, uint64_t end,
                      uint64_t tsid, uint64_t tsid_end);
void SyscallSplitTime(uint64_t cd0, uint64_t cd1, uint64_t cs,
                      uint64_t mid_quantum, uint64_t mid_id);

/* Channels */
void SyscallSliceChannels(uint64_t dest, uint64_t src, uint64_t begin,
                          uint64_t end);
void SyscallSplitChannels(uint64_t dest0, uint64_t dest1, uint64_t src,
                          uint64_t mid);
/* Endpoints */
void SyscallMakeReceiver(uint64_t cid_recv, uint64_t cid_ch, uint64_t epid);
void SyscallMakeSender(uint64_t cid_send, uint64_t cid_recv);
void SyscallRecv(uint64_t cid_recv, uint64_t caps_dest);
void SyscallSend(uint64_t cid_send, uint64_t caps_src,
                         uint64_t is_blocking, uint64_t m0, uint64_t m1,
                         uint64_t m2, uint64_t m3);
/* Supervisor */
void SyscallHalt(uint64_t cid_sup);
void SyscallResume(uint64_t cid_sup);
void SyscallGiveCap(uint64_t cid_sup, uint64_t cid_dest, uint64_t cid_src);
void SyscallTakeCap(uint64_t cid_sup, uint64_t cid_dest, uint64_t cid_src);
void SyscallSupReadCap(uint64_t cid_sup, uint64_t cid);
void SyscallReset(uint64_t cid_sup, uint64_t cid_pmp);

static inline Cap *proc_get_cap(Proc *p, uint64_t idx) {
        return &p->cap_table[idx % N_CAPS];
}

static inline Cap *curr_get_cap(uint64_t idx) {
        return proc_get_cap(current, idx);
}
