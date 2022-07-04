// See LICENSE file for copyright and license details.

#include "sched.h"
#include "syscall_nr.h"
#include "trap.h"
#include "types.h"
#include "utils.h"

void syscall_handler(TrapFrame *tf);

static void __syscall_no_cap(TrapFrame *tf);
static void __syscall_read_cap(TrapFrame *tf);
static void __syscall_move_cap(TrapFrame *tf);
static void __syscall_delete_cap(TrapFrame *tf);
static void __syscall_revoke_cap(TrapFrame *tf);
static void __syscall_derive_cap(TrapFrame *tf);
static void __syscall_invoke_cap(TrapFrame *tf);

/* Invoke functions */
static void __syscall_invoke_pmp(TrapFrame *tf, Cap cap, CapNode *cn);
static void __syscall_invoke_endpoint(TrapFrame *tf, Cap cap, CapNode *cn);
static void __syscall_invoke_supervisor(TrapFrame *tf, Cap cap, CapNode *cn);

static void (*const __syscall_handler_array[])(TrapFrame *) = {
    __syscall_no_cap,     __syscall_read_cap,   __syscall_move_cap,
    __syscall_delete_cap, __syscall_revoke_cap, __syscall_derive_cap,
    __syscall_invoke_cap};

void syscall_handler(TrapFrame *tf) {
        uint64_t syscall_number = tf->t0;
        if (syscall_number < ARRAY_SIZE(__syscall_handler_array)) {
                tf->pc += 4;
                __syscall_handler_array[syscall_number](tf);
        }
}

void __syscall_no_cap(TrapFrame *tf) {
        switch (tf->a0) {
                case 0:
                        /* Get the Process ID */
                        tf->a0 = current->pid;
                        break;
                default:
                        tf->a0 = 0;
        }
}

void __syscall_read_cap(TrapFrame *tf) {
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

void __syscall_move_cap(TrapFrame *tf) {
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

void __syscall_delete_cap(TrapFrame *tf) {
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
                        break;
                case CAP_TIME:
                        /* Unset time here */
                        SchedDelete(cap, cn);
                        break;
                default:
                        break;
        }
        tf->a0 = CapDelete(cn);
}

void __syscall_revoke_cap(TrapFrame *tf) {
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
                        break;
                case CAP_MEMORY:
                        cap_memory_set_free(&cap, cap_memory_get_begin(cap));
                        cap_memory_set_pmp(&cap, false);
                        break;
                case CAP_SUPERVISOR:
                        cap_supervisor_set_free(&cap, cap_supervisor_get_begin(cap));
                        break;
                case CAP_CHANNELS:
                        cap_channels_set_free(&cap, cap_channels_get_begin(cap));
                        cap_channels_set_ep(&cap, false);
                        break;
                default:
                        break;
        }
        tf->a0 = CapRevoke(cn) && CapUpdate(cap, cn);
}

void __syscall_derive_cap(TrapFrame *tf) {
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

void __syscall_invoke_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                tf->a0 = 0;
                return;
        }
        CapNode *cn = &current->cap_table[cid];
        Cap cap = cn_get(cn);
        switch (cap_get_type(cap)) {
                case CAP_PMP:
                        __syscall_invoke_pmp(tf, cap, cn);
                        break;
                case CAP_ENDPOINT:
                        __syscall_invoke_endpoint(tf, cap, cn);
                        break;
                case CAP_SUPERVISOR:
                        __syscall_invoke_supervisor(tf, cap, cn);
                        break;
                default:
                        tf->a0 = 0;
                        break;
        }
}

void __syscall_invoke_pmp(TrapFrame *tf, Cap cap, CapNode *cn) {
        uint64_t index = tf->a1;
        if (index >= N_PMP) {
                tf->a0 = 0;
        } else {
                tf->a0 = ProcLoadPmp(current, cap, cn, index);
        }
}

void __syscall_invoke_endpoint(TrapFrame *tf, Cap cap, CapNode *cn) {
}

void __syscall_invoke_supervisor(TrapFrame *tf, Cap cap, CapNode *cn) {
        uint64_t pid = tf->a1;
        /* Check free <= pid < end */
        uint64_t pid_free = cap_supervisor_get_free(cap);
        uint64_t pid_end = cap_supervisor_get_end(cap);
        if (pid < pid_free || pid >= pid_end) {
                tf->a0 = 0;
                return;
        }

        // Proc *supervisee = &processes[pid];
}

