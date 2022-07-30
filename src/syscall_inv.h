// See LICENSE file for copyright and license details.
#pragma once

#include "cap.h"
#include "cap_util.h"
#include "config.h"
#include "csr.h"
#include "proc.h"
#include "sched.h"
#include "syscall.h"
#include "syscall_nr.h"
#include "trap.h"

// TODO: fix return values in this file. 
// Currently functions typically return an unsigned int but can also return -1, which is not very intuitive. 

// TODO: since we have definitions in a header file all functions should probably be static, or
// we should move the definitions to a .c file.

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

Proc *volatile channels[N_CHANNELS];

static bool syscall_interprocess_move(Cap *src, Cap *dest, uint64_t old_pid, uint64_t new_pid) {
        CapData cd = cap_get(src);
        CapType type = cap_get_type(cd);
        bool succ;

        if (type == CAP_TIME_SLICE) {
                // TODO: add pids to caps and update it here
                CapTimeSlice ts = cap_deserialize_time_slice(cd);
                succ = CapMove(dest, src) &&
                       SchedUpdateAssumeNoPreemption(ts.begin, ts.end, ts.hartid, (ts.tsid << 8) | (uint8_t)old_pid,
                                   (ts.tsid << 8) | (uint8_t)new_pid, dest);
        } else {
                succ = CapMove(dest, src);
        }

        return succ;
}

static bool syscall_yield() {
        sys_to_sched();
        /* We never return here, but sys_to_sched will set the return value to true */
}

/*** MEMORY SLICE HANDLE ***/
static inline uint64_t ms_slice(const CapMemorySlice ms, Cap *parent,
                                Cap *child, uint64_t begin, uint64_t end,
                                uint64_t rwx) {
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

static inline uint64_t ms_split(const CapMemorySlice ms, Cap *parent,
                                Cap *child0, Cap *child1, uint64_t mid) {
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

static inline uint64_t ms_instanciate(const CapMemorySlice ms, Cap *parent,
                                      Cap *child, uint64_t addr, uint64_t rwx) {
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

static inline uint64_t pe_load(const CapPmpEntry pe, Cap *parent,
                               uint8_t index) {
        if (index > 8)
                return -1;
        Cap *child = &current->pmp_table[index];
        CapLoadedPmp lp_child = cap_mk_loaded_pmp(pe.addr, pe.rwx);
        if (!cap_set(child, cap_serialize_loaded_pmp(lp_child)))
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
                case SYSNR_READ_CAP:
                        /* Read cap */
                        return cap_get_arr(cap, &current->args[1]);
                case SYSNR_MOVE_CAP:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case SYSNR_DELETE_CAP:
                        /* Delete cap */
                        /* Revoke unloads PMP by deleting a CapLoadedPmp */
                        CapRevoke(cap);
                        return CapDelete(cap);
                case SYSNR_PE_UNLOAD:
                        /* Unload cap */
                        return CapRevoke(cap);
                case SYSNR_PE_LOAD:
                        /* Load cap */
                        return ProcLoadPmp(current, pe, cap, a1);
                default:
                        return -1;
        }
}

/*** TIME SLICE HANDLE ***/
static inline uint64_t ts_slice(const CapTimeSlice ts, Cap *parent, Cap *child,
                                uint64_t begin, uint64_t end, uint64_t tsid,
                                uint64_t tsid_end) {
        /* Check validity of time slice */
        if (!(begin < end && tsid < tsid_end))
                return 0;
        /* Check containment */
        if (!(ts.begin <= begin && end <= ts.end && ts.tsid < tsid &&
              tsid_end <= ts.tsid_end))
                return 0;
        /* If the parent already has a child it can't have another one */
        if (cap_is_child_ts(ts, cap_get(parent->next)))
                return 0;
        CapTimeSlice ts_child =
            cap_mk_time_slice(ts.hartid, begin, end, tsid, tsid_end);
        // TODO: can we find a situation where the order of CapAppend and SchedUpdate matter?
        return cap_set(child, cap_serialize_time_slice(ts_child)) &&
               CapAppend(child, parent) && 
               SchedUpdate(begin, end, ts.hartid, (ts.tsid << 8) | (uint8_t)current->pid, 
                           (tsid << 8) | (uint8_t)current->pid, parent);
}

static inline uint64_t ts_split(const CapTimeSlice ts, Cap *parent, Cap *child0,
                                Cap *child1, uint64_t qmid, uint64_t tsmid) {
        /* Check that qmid and tsmid are in middle of parents time slice and
         * tsid intervals */
        if (!(ts.begin < qmid && qmid < ts.end && (ts.tsid + 1) < tsmid &&
              tsmid + 1 < ts.tsid_end))
                return -1;
        if (cap_is_child_ts(ts, cap_get(parent->next)) || child0 == child1)
                return -1;
        CapTimeSlice ts_child0 = cap_mk_time_slice(ts.hartid, ts.begin, qmid, 
                                                   ts.tsid + 1, tsmid);
        CapTimeSlice ts_child1 = cap_mk_time_slice(ts.hartid, qmid + 1, ts.end,
                                                   tsmid + 1, ts.tsid_end);
        // TODO: can we find a situation where the order of CapAppend and SchedUpdate matter?
        return cap_set(child0, cap_serialize_time_slice(ts_child0)) &&
               cap_set(child1, cap_serialize_time_slice(ts_child1)) &&
               CapAppend(child0, parent) && CapAppend(child1, parent) &&
               SchedUpdate(ts_child0.begin, ts_child0.end, ts_child0.hartid, (ts.tsid << 8) | (uint8_t)current->pid, 
                           (ts_child0.tsid << 8) | (uint8_t)current->pid, parent) &&
               SchedUpdate(ts_child1.begin, ts_child1.end, ts_child1.hartid, (ts.tsid << 8) | (uint8_t)current->pid, 
                           (ts_child1.tsid << 8) | (uint8_t)current->pid, parent);
}

#if TIME_SLOT_LOANING_SIMPLE != 0
        static inline bool syscall_loan_time(uint64_t pid) {
                // TODO: do we want to give the user the option not to yield? 
                return SchedTryLoanTime(pid) && syscall_yield();
        }
        
        static bool syscall_return_loaned_time() {
                SchedReturnTime();
                // TODO: do we want to give the user the option not to yield? 
                return syscall_yield();
        }
#endif

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

                        /* If doing SchedDelete before CapDelete we might remove our only time slot and not have enough time to complete
                           CapDelete before being descheduled. 
                           
                           To allow CapDelete to be performed before SchedDelete we must allow SchedDelete to perform its operation 
                           even if the cap is removed. This means that it will still try to delete the cap from the schedule even if 
                           another revoke already has done it. This is not a problem since the expected value will never match in
                           this situation and the SchedDelete will simply do nothing. */
                        return CapDelete(cap) && SchedDelete(ts.begin, ts.end, ts.hartid, (ts.tsid << 8) | ((uint8_t)current->pid));
                case SYSNR_REVOKE_CAP:
                        /* Revoke time slice */
                        // TODO: can we find a situation where the order matters?
                        return SchedRevoke(ts.begin, ts.end, ts.hartid, (ts.tsid << 8) | ((uint8_t)current->pid), cap) && CapRevoke(cap);
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

/*** CHANNELS HANDLE ***/
uint64_t ch_slice(const CapChannels ch, Cap *parent, Cap *child, uint64_t begin,
                  uint64_t end) {
        if (cap_is_child_ch(ch, cap_get(parent->next)))
                return -1;
        if (begin > end)
                return -1;
        CapChannels ch_child = cap_mk_channels(begin, end);
        if (cap_is_child_ch_ch(ch, ch_child))
                return -1;
        return cap_set(child, cap_serialize_channels(ch_child)) &&
               CapAppend(child, parent);
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
        return cap_set(child0, cap_serialize_channels(ch_child0)) &&
               cap_set(child1, cap_serialize_channels(ch_child1)) &&
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
        return cap_set(child0, cap_serialize_receiver(recv_child0)) &&
               cap_set(child1, cap_serialize_sender(send_child1)) &&
               CapAppend(child0, parent) && CapAppend(child1, parent);
}

uint64_t SyscallChannels(const CapChannels ch, Cap *cap, uint64_t a1,
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
                        /* Delete channels */
                        return CapDelete(cap);
                case SYSNR_REVOKE_CAP:
                        /* Revoke all channels  */
                        return CapRevoke(cap);
                case SYSNR_CH_SLICE:
                        /* Slice channels */
                        return ch_slice(ch, cap, curr_get_cap(a1), a2, a3);
                case SYSNR_CH_SPLIT:
                        /* Split channels */
                        return ch_split(ch, cap, curr_get_cap(a1),
                                        curr_get_cap(a2), a3);
                case SYSNR_CH_INSTANCIATE:
                        /* Instanciate channels, make receiver and sender */
                        return ch_instanciate(ch, cap, curr_get_cap(a1),
                                              curr_get_cap(a2), a3);
                default:
                        return -1;
        }
}

uint64_t sn_recv(uint64_t channel) {
        current->listen_channel = channel;
        current->args[0] = false;
        if (__sync_bool_compare_and_swap(&channels[channel], NULL, current)) {
                trap_recv_yield();
        } else {
                current->listen_channel = -1;
        }
        /* We just wait to receive a message, sender does all the job */
        return current->args[0];
}

uint64_t sn_recv_delete_ts(uint64_t channel, Cap * cap_ts) {
        current->listen_channel = channel;
        current->args[0] = false;
        CapTimeSlice ts = cap_deserialize_time_slice(cap_get(cap_ts));
        
        if (__sync_bool_compare_and_swap(&channels[channel], NULL, current)) {
                uint64_t ret = CapDelete(cap_ts);
                if (!ret) 
                        return ret;
                /* We need to disable preemption during both the delete and updating our state. Both of them
                   prevents us from being scheduled anew, so if we are descheduled between them we will no be able to perform the other. 
                   The delete must happen first since it will temporarily enable preemption if it cant get the lock. */
                clear_csr(mstatus, 8);
                ret = ret && SchedDeleteAssumeNoPreemption(ts.begin, ts.end, ts.hartid, (ts.tsid << 8) | ((uint8_t)current->pid))
                          && __sync_bool_compare_and_swap(&current->state, PROC_RUNNING, PROC_WAITING);
                
                set_csr(mstatus, 8);
                if (!ret) 
                        return ret;
                sys_to_sched();
                /* We never return here, but sys_to_sched will set the return value to true */
        } else {
                current->listen_channel = -1;
        }
        /* We just wait to receive a message, sender does all the job */
        return current->args[0];
}

/*** RECEIVER HANDLER ***/
uint64_t SyscallReceiver(const CapReceiver receiver, Cap *cap, uint64_t a1,
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
                        return CapDelete(cap);
                case SYSNR_RC_RECEIVE:
                        /* Receive message */
                        return sn_recv(receiver.channel);
                case SYSNR_RC_RECEIVE_DELETE_TS:
                        /* Receive message and delete a timeslice before yielding */
                        Cap *cap_ts = curr_get_cap(a4);
                        CapData cd = cap_get(cap_ts);
                        if (cap_get_type(cd) != CAP_TIME_SLICE) 
                                return 0;
                        return sn_recv_delete_ts(receiver.channel, cap_ts);
                default:
                        return -1;
        }
}

uint64_t sn_send(uint64_t channel) {
        Proc *receiver;
        /* Acquire the receiver in the channel */
        bool acquired_channel = false;
        do {
                receiver = channels[channel];
                if (receiver == NULL) {
                        /* No receiver */
                        current->args[0] = false;
                        return current->args[0];
                }
                /* Receiver is not waiting */
                if (receiver->state != PROC_WAITING)
                        continue;
                acquired_channel = __sync_bool_compare_and_swap(&channels[channel], receiver, NULL);
        } while (!acquired_channel);
        /* Acquire the receiver state */
        if (!__sync_bool_compare_and_swap(&receiver->state, PROC_WAITING, PROC_RECEIVING)) {
                current->args[0] = false;
                return current->args[0];
        }

        /* From where to get capabilities */
        uint64_t cid_src = current->args[1];
        /* From where to set capabilities */
        uint64_t cid_dest = receiver->args[1];
        /* Number of capabilities to send */
        uint64_t n = (current->args[2] > receiver->args[2]) ? receiver->args[2] : current->args[2];
        /* Position of last capability to send (excl.) */
        uint64_t cid_last = cid_src + n;
        /* Number of capabilities sent */
        current->args[1] = 0;
        /* We need to diable preemption here, otherwise it's possible to give away all our time and then be descheduled before reaching
           the line where we set our receiver to suspended, letting it get scheduled. */
        clear_csr(mstatus, 8);
        while (cid_src < N_CAPS && cid_src < cid_last && cid_dest < N_CAPS) {
                Cap *src = &current->cap_table[cid_src];
                Cap *dest = &receiver->cap_table[cid_dest];
                if(!syscall_interprocess_move(src, dest, current->pid, receiver->pid)) {
                        /* If moving a capability failed */
                        break;
                }
                cid_src++;
                cid_dest++;
                current->args[1]++;
        }
        /* Send successful */
        current->args[0] = true;
        receiver->args[0] = true;

        /* Number of caps sent */
        receiver->args[1] = current->args[1];
        /* Message passing, 256 bits */
        uint64_t *send = (uint64_t *)current->args[3];
        uint64_t *recv = (uint64_t *)receiver->args[3];
        recv[0] = send[0];
        recv[1] = send[1];
        recv[2] = send[2];
        recv[3] = send[3];

        /* Ensure the the receiver can see the messages before allowing it to be scheduled, in case it's on another hart. */
        __sync_synchronize();
        /* Sets the receiver to suspended */
        receiver->state = PROC_SUSPENDED;
        set_csr(mstatus, 8);
        return current->args[0];
}

/*** SENDER HANDLER ***/
uint64_t SyscallSender(const CapSender sender, Cap *cap, uint64_t a1,
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
                        return CapDelete(cap);
                case SYSNR_SN_SEND:
                        /* Send message */
                        return sn_send(sender.channel);
                default:
                        return -1;
        }
}

static inline uint64_t sup_halt(Proc *supervisee) {
        /* Check if supervisee has halted or is to halt */
        if (supervisee->state == PROC_HALTED || supervisee->halt)
                return false;
        ProcHalt(supervisee);
        return true;
}

static inline uint64_t sup_is_halted(Proc *supervisee) {
        return supervisee->state == PROC_HALTED;
}

static inline uint64_t sup_resume(Proc *supervisee) {
        if (supervisee->state == PROC_HALTED) {
                supervisee->state = PROC_SUSPENDED;
                return true;
        }
        return false;
}

static inline uint64_t sup_reset(Proc *supervisee, Cap *pmp_cap) {
        CapData cd = cap_get(pmp_cap);
        if (cap_get_type(cd) != CAP_PMP_ENTRY)
                return -1;
        CapPmpEntry pe = cap_deserialize_pmp_entry(cd);
        if (supervisee->state != PROC_HALTED)
                return -1;
        /* Reset the process */
        ProcReset(supervisee->pid);
        return CapMove(&supervisee->cap_table[0], pmp_cap) &&
               ProcLoadPmp(supervisee, pe, pmp_cap, 0);
}

static inline uint64_t sup_read(Proc *supervisee, uint64_t cid) {
        return cap_get_arr(proc_get_cap(supervisee, cid), &current->args[1]);
}

static inline uint64_t sup_give(Proc *supervisee, uint64_t cid_dest,
                                uint64_t cid_src) {
        return 0;
}

static inline uint64_t sup_take(Proc *supervisee, uint64_t cid_dest,
                                uint64_t cid_src) {
        return 0;
}

/*** SUPERVISOR HANDLER ***/
uint64_t SyscallSupervisor(const CapSupervisor sup, Cap *cap, uint64_t a1,
                           uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5,
                           uint64_t a6, uint64_t a7) {
        Proc *supervisee = &processes[sup.pid];
        switch (a7) {
                case SYSNR_READ_CAP:
                        /* Read cap */
                        return cap_get_arr(cap, &current->args[1]);
                case SYSNR_MOVE_CAP:
                        /* Move cap */
                        return CapMove(curr_get_cap(a1), cap);
                case SYSNR_DELETE_CAP:
                        /* Delete time slice */
                        return CapDelete(cap);
                case SYSNR_SP_IS_HALTED:
                        return sup_is_halted(supervisee);
                case SYSNR_SP_HALT:
                        /* Halt process */
                        return sup_halt(supervisee);
                case SYSNR_SP_RESUME:
                        /* Resume process */
                        return sup_resume(supervisee);
                case SYSNR_SP_RESET:
                        /* Reset process */
                        return sup_reset(supervisee, curr_get_cap(a1));
                case SYSNR_SP_READ:
                        /* Read cap */
                        return sup_read(supervisee, a1);
                case SYSNR_SP_GIVE:
                        /* Give cap */
                        return sup_give(supervisee, a1, a2);
                case SYSNR_SP_TAKE:
                        /* Take cap */
                        return sup_take(supervisee, a1, a2);
                default:
                        return -1;
        }
}
