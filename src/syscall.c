// See LICENSE file for copyright and license details.

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
static void syscall_invoke_pmp(TrapFrame *tf, Cap cap, CapNode *cn);
static void syscall_invoke_endpoint(TrapFrame *tf, Cap cap, CapNode *cn);
static void syscall_invoke_supervisor(TrapFrame *tf, Cap cap, CapNode *cn);

static void (*const syscall_handler_array[])(TrapFrame *) = {
    syscall_no_cap,     syscall_read_cap,   syscall_move_cap,
    syscall_delete_cap, syscall_revoke_cap, syscall_derive_cap,
    syscall_invoke_cap};

void syscall_handler(TrapFrame *tf) {
        uint64_t syscall_number = tf->t0;
        if (syscall_number < ARRAY_SIZE(syscall_handler_array)) {
                tf->pc += 4;
                syscall_handler_array[syscall_number](tf);
        }
}

void syscall_no_cap(TrapFrame *tf) {
        switch (tf->a0) {
                case 0:
                        /* Get the Process ID */
                        tf->a0 = current->pid;
                        break;
                default:
                        tf->a0 = 0;
        }
}

void syscall_read_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = 0;
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
                tf->a0 = 0;
                return;
        }
        CapNode *cn_src = &current->cap_table[cid_src];
        CapNode *cn_dest = &current->cap_table[cid_dest];
        tf->a0 = CapMove(cn_src, cn_dest);
}

void syscall_delete_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = 0;
                return;
        }
        CapNode *cn = &current->cap_table[cid];
        const Cap cap = cn_get(cn);
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
}

void syscall_revoke_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = 0;
                return;
        }
        CapNode *cn = &current->cap_table[cid];
        Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_TIME:
                        SchedRevoke(cap, cn);
                        cap_time_set_free(&cap, cap_time_get_begin(cap));
                        tf->a0 = CapRevoke(cn) && SchedRevoke(cap, cn) && CapUpdate(cap, cn);
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
                        tf->a0 = false;
                        break;
        }
}

void syscall_derive_cap(TrapFrame *tf) {
        uint64_t cid_src = tf->a0;
        uint64_t cid_dest = tf->a1;
        uint64_t word0 = tf->a2;
        uint64_t word1 = tf->a3;
        if (cid_src >= N_CAPS || cid_dest >= N_CAPS || cid_src == cid_dest) {
                tf->a0 = 0;
                return;
        }

        CapNode *cn_parent = &current->cap_table[cid_src];
        CapNode *cn_child = &current->cap_table[cid_dest];

        Cap parent = cn_get(cn_parent);
        Cap child = (Cap){word0, word1};

        CapType parent_type = cap_get_type(parent);
        CapType child_type = cap_get_type(child);

        if (!cap_can_derive(parent, child)) {
                tf->a0 = 0;
                return;
        }

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
                         SchedUpdate(child, parent, cn_parent);
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
}

void syscall_invoke_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = 0;
                return;
        }
        CapNode *cn = &current->cap_table[cid];
        Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_PMP:
                        syscall_invoke_pmp(tf, cap, cn);
                        break;
                case CAP_ENDPOINT:
                        syscall_invoke_endpoint(tf, cap, cn);
                        break;
                case CAP_SUPERVISOR:
                        syscall_invoke_supervisor(tf, cap, cn);
                        break;
                default:
                        tf->a0 = 0;
                        break;
        }
}

void syscall_invoke_pmp(TrapFrame *tf, Cap cap, CapNode *cn) {
        uint64_t index = tf->a1;
        if (index >= N_PMP) {
                tf->a0 = 0;
        } else {
                tf->a0 = ProcLoadPmp(current, cap, cn, index);
        }
}

void syscall_invoke_endpoint(TrapFrame *tf, Cap cap, CapNode *cn) {
}

bool syscall_invoke_supervisor_halt(Proc *supervisee) {
        supervisee->halt = true;
        __sync_bool_compare_and_swap(&supervisee->state, PROC_SUSPENDED,
                                     PROC_HALTED);
        return true;
}

bool syscall_invoke_supervisor_resume(Proc *supervisee) {
        supervisee->halt = false;
        __sync_bool_compare_and_swap(&supervisee->state, PROC_HALTED,
                                     PROC_SUSPENDED);
        return true;
}

bool syscall_invoke_supervisor_halted(Proc *supervisee) {
        return supervisee->state == PROC_HALTED;
}

void syscall_invoke_supervisor(TrapFrame *tf, Cap cap, CapNode *cn) {
        uint64_t pid = tf->a1;
        uint64_t op = tf->a2;
        /* Check free <= pid < end */
        uint64_t pid_free = cap_supervisor_get_free(cap);
        uint64_t pid_end = cap_supervisor_get_end(cap);
        if (pid < pid_free || pid >= pid_end) {
                tf->a0 = 0;
                return;
        }

        Proc *supervisee = &processes[pid];
        switch (op) {
                case 0:
                        tf->a0 = syscall_invoke_supervisor_halt(supervisee);
                        break;
                case 1:
                        tf->a0 = syscall_invoke_supervisor_resume(supervisee);
                        break;
                case 2: 
                        tf->a0 = syscall_invoke_supervisor_halted(supervisee);
                        break;
                case 3: 

                default:
                        tf->a0 = false;
                        break;
        }
}

