// See LICENSE file for copyright and license details.

#include "syscall.h"

#include "cap_util.h"
#include "syscall_inv.h"

uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
        Cap *cap = curr_get_cap(a0);
        CapUnion cu = cap_get(cap);
        switch (cu.type) {
                case CAP_MEMORY_SLICE:
                        return SyscallMemorySlice(cu.memory_slice, cap, a1, a2,
                                                  a3, a4, a5, a6, a7);
                case CAP_PMP_ENTRY:
                        return SyscallPmpEntry(cu.pmp_entry, cap, a1, a2, a3,
                                               a4, a5, a6, a7);
                case CAP_TIME_SLICE:
                        return SyscallTimeSlice(cu.time_slice, cap, a1, a2, a3,
                                                a4, a5, a6, a7);
                case CAP_CHANNELS:
                        return SyscallChannels(cu.channels, cap, a1, a2, a3, a4,
                                               a5, a6, a7);
                case CAP_RECEIVER:
                        return SyscallReceiver(cu.receiver, cap, a1, a2, a3, a4,
                                               a5, a6, a7);
                case CAP_SENDER:
                        return SyscallSender(cu.sender, cap, a1, a2, a3, a4, a5,
                                             a6, a7);
                case CAP_SUPERVISOR:
                        return SyscallSupervisor(cu.supervisor, cap, a1, a2, a3,
                                                 a4, a5, a6, a7);
                default:
                        return -1;
        }
}

uint64_t SyscallNoCap(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4,
                      uint64_t a5, uint64_t a6, uint64_t a7) {
        switch (a7) {
                case 0:
                        /* Get the first PMP */
                        return cap_get_data(curr_get_cap(0), &current->args[1]);
                case 1:
                        /* Get the Process ID */
                        return current->pid;
                default:
                        return -1;
        }
}

void SyscallHandler(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
        if (a0 == 0) {
                current->args[0] = SyscallNoCap(a1, a2, a3, a4, a5, a6, a7);
        } else {
                current->args[0] = SyscallCap(a0, a1, a2, a3, a4, a5, a6, a7);
        }
}
