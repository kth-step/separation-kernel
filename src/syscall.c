// See LICENSE file for copyright and license details.

#include "syscall.h"

static uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                           uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7);
static uint64_t SyscallNoCap(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                             uint64_t a5, uint64_t a6, uint64_t a7);

static uint64_t SyscallChannels(const Cap cap, CapNode *cn, uint64_t a1,
                                uint64_t a2, uint64_t a3, uint64_t a4,
                                uint64_t a5, uint64_t a6, uint64_t a7);
static uint64_t SyscallEndpoint(const Cap cap, CapNode *cn, uint64_t a1,
                                uint64_t a2, uint64_t a3, uint64_t a4,
                                uint64_t a5, uint64_t a6, uint64_t a7);
static uint64_t SyscallMemorySlice(const Cap cap, CapNode *cn, uint64_t a1,
                                   uint64_t a2, uint64_t a3, uint64_t a4,
                                   uint64_t a5, uint64_t a6, uint64_t sysnr);
static uint64_t SyscallPmpEntry(const Cap cap, CapNode *cn, uint64_t a1,
                                uint64_t a2, uint64_t a3, uint64_t a4,
                                uint64_t a5, uint64_t a6, uint64_t a7);
static uint64_t SyscallSupervisor(const Cap cap, CapNode *cn, uint64_t a1,
                                  uint64_t a2, uint64_t a3, uint64_t a4,
                                  uint64_t a5, uint64_t a6, uint64_t a7);
static uint64_t SyscallTimeSlice(const Cap cap, CapNode *cn, uint64_t a1,
                                 uint64_t a2, uint64_t a3, uint64_t a4,
                                 uint64_t a5, uint64_t a6, uint64_t a7);

/* Include the implementations of each capability syscall */
#include "syscall/channels.c"
#include "syscall/endpoint.c"
#include "syscall/memory_slice.c"
#include "syscall/pmp_entry.c"
#include "syscall/supervisor.c"
#include "syscall/time_slice.c"

void SyscallHandler(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
        if (a0 == 0) {
                current->args[0] = SyscallNoCap(a1, a2, a3, a4, a5, a6, a7);
        } else {
                current->args[0] = SyscallCap(a0, a1, a2, a3, a4, a5, a6, a7);
        }
}

uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
        CapNode *cn = curr_get_cn(a0);
        const Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_TYPE_MEMORY_SLICE:
                        return SyscallMemorySlice(cap, cn, a1, a2, a3, a4, a5,
                                                  a6, a7);
                case CAP_TYPE_PMP_ENTRY:
                        return SyscallPmpEntry(cap, cn, a1, a2, a3, a4, a5, a6,
                                               a7);
                case CAP_TYPE_TIME_SLICE:
                        return SyscallTimeSlice(cap, cn, a1, a2, a3, a4, a5, a6,
                                                a7);
                case CAP_TYPE_CHANNELS:
                        return SyscallChannels(cap, cn, a1, a2, a3, a4, a5, a6,
                                               a7);
                case CAP_TYPE_ENDPOINT:
                        return SyscallEndpoint(cap, cn, a1, a2, a3, a4, a5, a6,
                                               a7);
                case CAP_TYPE_SUPERVISOR:
                        return SyscallSupervisor(cap, cn, a1, a2, a3, a4, a5,
                                                 a6, a7);
                default:
                        return -1;
        }
}

uint64_t SyscallNoCap(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                      uint64_t a5, uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        /* Get the first PMP */
                        Cap cap = cn_get(curr_get_cn(0));
                        current->args[1] = cap.word0;
                        current->args[2] = cap.word1;
                        return 1;
                case 1:
                        /* Get the Process ID */
                        return current->pid;
                case 2:
                        /* Unload PMP entry at a1, but not entry 0 */
                        if (a1 > 0 && a1 < 8)
                                return ProcUnloadPmp(current, a1);
                        else
                                return -1;
                default:
                        return -1;
        }
}
