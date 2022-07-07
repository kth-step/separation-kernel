// See LICENSE file for copyright and license details.

#include "csr.h"
#include "kprint.h"
#include "sched.h"
#include "syscall_nr.h"
#include "trap.h"
#include "types.h"
#include "utils.h"

void syscall_handler(TrapFrame *tf);

static void syscall_no_cap(TrapFrame *tf);
static void syscall_read_cap(TrapFrame *tf);
static void syscall_move_cap(TrapFrame *tf);
static void syscall_delete_cap(TrapFrame *tf);
static void syscall_revoke_cap(TrapFrame *tf);
static void syscall_derive_cap(TrapFrame *tf);
static void syscall_invoke_cap(TrapFrame *tf);

/* Invoke functions */
static void syscall_invoke_endpoint(TrapFrame *tf, Cap cap, CapNode *cn);
static void syscall_invoke_supervisor(TrapFrame *tf, Cap cap, CapNode *cn);

static void (*const syscall_handler_array[])(TrapFrame *) = {
    syscall_no_cap,     syscall_read_cap,   syscall_move_cap,
    syscall_delete_cap, syscall_revoke_cap, syscall_derive_cap,
    syscall_invoke_cap};

void SyscallHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval) {
        uint64_t syscall_number = tf->t0;
        if (syscall_number < ARRAY_SIZE(syscall_handler_array)) {
                tf->pc += 4;
                syscall_handler_array[syscall_number](tf);
        } else {
                ExceptionHandler(tf, mcause, mtval);
        }
}

void syscall_no_cap(TrapFrame *tf) {
        switch (tf->a0) {
                case 0:
                        /* Get the Process ID */
                        tf->a0 = current->pid;
                        break;
                default:
                        tf->a0 = -1;
        }
}

void syscall_read_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = -1;
                return;
        }
        const Cap cap = cn_get(&current->cap_table[cid]);
        tf->a0 = cap.word0 != 0;
        tf->a1 = cap.word0;
        tf->a2 = cap.word1;
}

void syscall_move_cap(TrapFrame *tf) {
        uint64_t cid_src = tf->a0;
        uint64_t cid_dest = tf->a1;
        if (cid_src >= N_CAPS || cid_dest >= N_CAPS || cid_src == cid_dest) {
                tf->a0 = -1;
                return;
        }
        CapNode *cn_src = &current->cap_table[cid_src];
        CapNode *cn_dest = &current->cap_table[cid_dest];
        SchedEnablePreemption();
        tf->a0 = CapMove(cn_src, cn_dest);
        SchedDisablePreemption();
}

void syscall_delete_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = -1;
                return;
        }
        CapNode *cn = &current->cap_table[cid];
        const Cap cap = cn_get(cn);
        SchedEnablePreemption();
        switch (cap_get_type(cap)) {
                case CAP_PMP:
                        CapRevoke(cn);
                        tf->a0 = CapDelete(cn);
                        break;
                case CAP_TIME:
                        /* Unset time here */
                        tf->a0 = CapDelete(cn) && SchedDelete(cap, cn);
                        break;
                default:
                        tf->a0 = CapDelete(cn);
                        break;
        }

        SchedDisablePreemption();
}

void syscall_revoke_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = -1;
                return;
        }
        CapNode *cn = &current->cap_table[cid];
        Cap cap = cn_get(cn);
        SchedDisablePreemption();
        switch (cap_get_type(cap)) {
                case CAP_PMP:
                        tf->a0 = CapRevoke(cn);
                        break;
                case CAP_TIME:
                        cap_time_set_free(&cap, cap_time_get_begin(cap));
                        tf->a0 = CapRevoke(cn) && SchedRevoke(cap, cn) &&
                                 CapUpdate(cap, cn);
                        break;
                case CAP_MEMORY:
                        cap_memory_set_free(&cap, cap_memory_get_begin(cap));
                        cap_memory_set_pmp(&cap, false);
                        tf->a0 = CapRevoke(cn) && CapUpdate(cap, cn);
                        break;
                case CAP_SUPERVISOR:
                        cap_supervisor_set_free(&cap,
                                                cap_supervisor_get_begin(cap));
                        tf->a0 = CapRevoke(cn) && CapUpdate(cap, cn);
                        break;
                case CAP_CHANNELS:
                        cap_channels_set_free(&cap,
                                              cap_channels_get_begin(cap));
                        cap_channels_set_ep(&cap, false);
                        tf->a0 = CapRevoke(cn) && CapUpdate(cap, cn);
                        break;
                default:
                        tf->a0 = -1;
                        break;
        }

        SchedDisablePreemption();
}

void syscall_derive_cap(TrapFrame *tf) {
        uint64_t cid_src = tf->a0;
        uint64_t cid_dest = tf->a1;
        uint64_t word0 = tf->a2;
        uint64_t word1 = tf->a3;
        if (cid_src >= N_CAPS || cid_dest >= N_CAPS || cid_src == cid_dest) {
                tf->a0 = -1;
                return;
        }

        CapNode *cn_parent = &current->cap_table[cid_src];
        CapNode *cn_child = &current->cap_table[cid_dest];

        Cap parent = cn_get(cn_parent);
        Cap child = (Cap){word0, word1};

        CapType parent_type = cap_get_type(parent);
        CapType child_type = cap_get_type(child);

        if (!cap_can_derive(parent, child)) {
                tf->a0 = -2;
                return;
        }

        SchedDisablePreemption();
        if (parent_type == CAP_MEMORY && child_type == CAP_MEMORY) {
                uint64_t end = cap_memory_get_begin(child);
                cap_memory_set_free(&parent, end);
                tf->a0 = CapUpdate(parent, cn_parent) &&
                         CapInsert(child, cn_child, cn_parent);
        } else if (parent_type == CAP_MEMORY && child_type == CAP_PMP) {
                cap_memory_set_pmp(&parent, true);
                tf->a0 = CapUpdate(parent, cn_parent) &&
                         CapInsert(child, cn_child, cn_parent);
        } else if (parent_type == CAP_TIME && child_type == CAP_TIME) {
                uint64_t end = cap_time_get_end(child);
                cap_time_set_free(&parent, end);
                tf->a0 = CapUpdate(parent, cn_parent) &&
                         CapInsert(child, cn_child, cn_parent) &&
                         SchedUpdate(parent, child, cn_parent);
        } else if (parent_type == CAP_CHANNELS && child_type == CAP_CHANNELS) {
                uint64_t end = cap_channels_get_end(child);
                cap_channels_set_free(&parent, end);
                tf->a0 = CapUpdate(parent, cn_parent) &&
                         CapInsert(child, cn_child, cn_parent);
        } else if (parent_type == CAP_CHANNELS && child_type == CAP_ENDPOINT) {
                cap_channels_set_ep(&parent, true);
                tf->a0 = CapUpdate(parent, cn_parent) &&
                         CapInsert(child, cn_child, cn_parent);
        } else if (parent_type == CAP_SUPERVISOR &&
                   child_type == CAP_SUPERVISOR) {
                uint64_t end = cap_supervisor_get_end(child);
                cap_supervisor_set_free(&parent, end);
                tf->a0 = CapUpdate(parent, cn_parent) &&
                         CapInsert(child, cn_child, cn_parent);
        }

        SchedEnablePreemption();
}

void syscall_invoke_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = -1;
                return;
        }
        CapNode *cn = &current->cap_table[cid];
        Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_ENDPOINT:
                        syscall_invoke_endpoint(tf, cap, cn);
                        break;
                case CAP_SUPERVISOR:
                        syscall_invoke_supervisor(tf, cap, cn);
                        break;
                default:
                        tf->a0 = -2;
                        break;
        }
}

void syscall_invoke_endpoint(TrapFrame *tf, Cap cap, CapNode *cn) {
}

void syscall_invoke_supervisor_halt(TrapFrame *tf, Proc *supervisee) {
        SchedDisablePreemption();
        tf->a0 = __sync_bool_compare_and_swap(&supervisee->state,
                                              PROC_SUSPENDED, PROC_HALTED) ||
                 __sync_bool_compare_and_swap(&supervisee->state, PROC_RUNNING,
                                              PROC_HALTING);

        SchedEnablePreemption();
}

void syscall_invoke_supervisor_resume(TrapFrame *tf, Proc *supervisee) {
        SchedDisablePreemption();
        tf->a0 = __sync_bool_compare_and_swap(&supervisee->state, PROC_HALTED,
                                              PROC_SUSPENDED);

        SchedEnablePreemption();
}

bool syscall_interprocess_move(CapNode *src, CapNode *dest, uint64_t pid) {
        Cap cap = cn_get(src);
        CapType type = cap_get_type(cap);
        bool succ;
        if (type == CAP_INVALID) {
                return false;
        }

        if (type == CAP_TIME) {
                Cap new_cap = cap;
                cap_time_set_pid(&new_cap, pid);
                succ = CapInsert(new_cap, dest, src) && CapDelete(src) &&
                       SchedUpdate(cap, new_cap, dest);
        } else if (type == CAP_PMP) {
                CapRevoke(src);
                succ = CapMove(src, dest);
        } else {
                succ = CapMove(src, dest);
        }
        return succ;
}

void syscall_invoke_supervisor_state(TrapFrame *tf, Proc *supervisee) {
        ProcState state;
        do {
                state = (volatile ProcState)supervisee->state;
        } while (state == PROC_BLOCKED);
        tf->a0 = state;
}

void syscall_invoke_supervisor_read_reg(TrapFrame *tf, Proc *supervisee) {
        SchedDisablePreemption();
        uint64_t reg_nr = tf->a3;
        if (reg_nr < PROC_NUM_OF_REGS &&
            __sync_bool_compare_and_swap(&supervisee->state, PROC_HALTED,
                                         PROC_BLOCKED)) {
                tf->a0 = 1;
                tf->a1 = ((uint64_t *)supervisee->tf)[reg_nr];
                __sync_synchronize();
                supervisee->state = PROC_HALTED;
        }
        SchedEnablePreemption();
}

void syscall_invoke_supervisor_write_reg(TrapFrame *tf, Proc *supervisee) {
        SchedDisablePreemption();
        uint64_t reg_nr = tf->a3;
        uint64_t reg_value = tf->a4;
        if (reg_nr < PROC_NUM_OF_REGS &&
            __sync_bool_compare_and_swap(&supervisee->state, PROC_HALTED,
                                         PROC_BLOCKED)) {
                tf->a0 = 1;
                ((uint64_t *)supervisee->tf)[reg_nr] = reg_value;
                __sync_synchronize();
                supervisee->state = PROC_HALTED;
        }
        SchedEnablePreemption();
}

void syscall_invoke_supervisor_give(TrapFrame *tf, Proc *supervisee) {
        uint64_t cid_src = tf->a3;
        uint64_t cid_dest = tf->a4;
        uint64_t cid_last = cid_src + tf->a5;
        tf->a0 = 0;
        bool succ;
        while (cid_src < N_CAPS && cid_src < cid_last && cid_dest < N_CAPS) {
                CapNode *src = &current->cap_table[cid_src];
                CapNode *dest = &supervisee->cap_table[cid_dest];
                SchedDisablePreemption();
                if (__sync_bool_compare_and_swap(&supervisee->state,
                                                 PROC_HALTED, PROC_BLOCKED)) {
                        succ = syscall_interprocess_move(src, dest,
                                                         supervisee->pid);
                        __sync_synchronize();
                        supervisee->state = PROC_HALTED;
                } else {
                        succ = false;
                }
                SchedEnablePreemption();
                if (!succ)
                        break;
                cid_src++;
                cid_dest++;
                tf->a0++;
        }
}

void syscall_invoke_supervisor_take(TrapFrame *tf, Proc *supervisee) {
        uint64_t cid_src = tf->a3;
        uint64_t cid_dest = tf->a4;
        uint64_t cid_last = cid_src + tf->a5;
        bool succ;
        tf->a0 = 0;
        while (cid_src < N_CAPS && cid_src < cid_last && cid_dest < N_CAPS) {
                CapNode *src = &supervisee->cap_table[cid_src];
                CapNode *dest = &current->cap_table[cid_dest];
                SchedDisablePreemption();
                if (__sync_bool_compare_and_swap(&supervisee->state,
                                                 PROC_HALTED, PROC_BLOCKED)) {
                        succ =
                            syscall_interprocess_move(src, dest, current->pid);
                } else {
                        succ = false;
                }
                SchedEnablePreemption();
                if (!succ)
                        break;
                cid_src++;
                cid_dest++;
                tf->a0++;
        }
}

void syscall_invoke_supervisor(TrapFrame *tf, Cap cap, CapNode *cn) {
        uint64_t pid = tf->a1;
        uint64_t op = tf->a2;
        /* Check free <= pid < end */
        uint64_t pid_free = cap_supervisor_get_free(cap);
        uint64_t pid_end = cap_supervisor_get_end(cap);
        if (pid < pid_free || pid >= pid_end) {
                tf->a0 = -3;
                return;
        }

        Proc *supervisee = &processes[pid];
        switch (op) {
                case 0:
                        syscall_invoke_supervisor_halt(tf, supervisee);
                        break;
                case 1:
                        syscall_invoke_supervisor_resume(tf, supervisee);
                        break;
                case 2:
                        syscall_invoke_supervisor_state(tf, supervisee);
                        break;
                case 3:
                        syscall_invoke_supervisor_read_reg(tf, supervisee);
                        break;
                case 4:
                        syscall_invoke_supervisor_write_reg(tf, supervisee);
                        break;
                case 5:
                        syscall_invoke_supervisor_give(tf, supervisee);
                        break;
                case 6:
                        syscall_invoke_supervisor_take(tf, supervisee);
                        break;
                default:
                        break;
        }
}

