// See LICENSE file for copyright and license details.

#include "syscall.h"

#include "cap_util.h"
#include "syscall_inv.h"

uint64_t SyscallCap(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7) {
        Cap *cap = curr_get_cap(a0);
        const CapData cd = cap_get(cap);
        switch (cap_get_type(cd)) {
                case CAP_MEMORY_SLICE:
                        return SyscallMemorySlice(cap_deserialize_memory_slice(cd), cap, a1, a2,
                                                  a3, a4, a5, a6, a7);
                case CAP_PMP_ENTRY:
                        return SyscallPmpEntry(cap_deserialize_pmp_entry(cd), cap, a1, a2, a3,
                                               a4, a5, a6, a7);
                case CAP_TIME_SLICE:
                        return SyscallTimeSlice(cap_deserialize_time_slice(cd), cap, a1, a2, a3,
                                                a4, a5, a6, a7);
                case CAP_CHANNELS:
                        return SyscallChannels(cap_deserialize_channels(cd), cap, a1, a2, a3, a4,
                                               a5, a6, a7);
                case CAP_RECEIVER:
                        return SyscallReceiver(cap_deserialize_receiver(cd), cap, a1, a2, a3, a4,
                                               a5, a6, a7);
                case CAP_SENDER:
                        return SyscallSender(cap_deserialize_sender(cd), cap, a1, a2, a3, a4, a5,
                                             a6, a7);
                case CAP_SUPERVISOR:
                        return SyscallSupervisor(cap_deserialize_supervisor(cd), cap, a1, a2, a3,
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
                        return cap_get_arr(curr_get_cap(0), &current->args[1]);
                case 1:
                        /* Get the Process ID */
                        return current->pid;
                case 2:
                        /* Unload PMP entry at a1, but not entry 0 */
                        return (a1 > 0) ? ProcUnloadPmp(current, a1) : 0;
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
