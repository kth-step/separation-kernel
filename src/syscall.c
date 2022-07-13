// See LICENSE file for copyright and license details.

#include "cap_utils.h"
#include "csr.h"
#include "s3k_consts.h"
#include "sched.h"
#include "stack.h"
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

/* Invoke functions */
static void syscall_invoke_endpoint(TrapFrame *tf);
static void syscall_invoke_supervisor(TrapFrame *tf);

static bool syscall_interprocess_move(CapNode *src, CapNode *dest,
                                      uint64_t pid);

static void (*const syscall_handler_array[])(TrapFrame *) = {
    syscall_no_cap,          syscall_read_cap,         syscall_move_cap,
    syscall_delete_cap,      syscall_revoke_cap,       syscall_derive_cap,
    syscall_invoke_endpoint, syscall_invoke_supervisor};

Proc *volatile channels[N_CHANNELS];

void SyscallHandler(TrapFrame *tf, uint64_t mcause, uint64_t mtval) {
        uint64_t syscall_number = tf->t0;
        if (syscall_number < ARRAY_SIZE(syscall_handler_array)) {
                syscall_handler_array[syscall_number](tf);
        } else {
                ExceptionHandler(tf, mcause, mtval);
        }
}

void syscall_no_cap(TrapFrame *tf) {
        uint64_t op_number = tf->a0;
        SchedDisablePreemption();
        switch (op_number) {
                case S3K_SYSNR_NOCAP_GET_PID:
                        /* Get the Process ID */
                        tf->a0 = S3K_ERROR_OK;
                        tf->a1 = current->pid;
                        break;
                case S3K_SYSNR_NOCAP_READ_REGISTER:
                        tf->a0 = S3K_ERROR_OK;
                        tf->a1 = proc_read_register(current, tf->a1);
                        break;
                case S3K_SYSNR_NOCAP_WRITE_REGISTER:
                        tf->a0 = S3K_ERROR_OK;
                        tf->a1 = proc_write_register(current, tf->a1, tf->a2);
                        break;
                default:
                        tf->a0 = S3K_ERROR_NOCAP_BAD_OP;
                        break;
        }
        tf->pc += 4;
        SchedEnablePreemption();
}

bool syscall_interprocess_move(CapNode *src, CapNode *dest, uint64_t pid) {
        Cap cap = cap_node_get_cap(src);
        CapType type = cap_get_type(cap);
        bool succ;

        if (type == CAP_TIME) {
                Cap new_cap = cap;
                cap_time_set_pid(&new_cap, pid);
                succ = CapInsert(new_cap, dest, src) && CapDelete(src) &&
                       SchedUpdate(cap, new_cap, dest);
        } else {
                succ = CapMove(src, dest);
        }

        return succ;
}

void syscall_read_cap(TrapFrame *tf) {
        uint64_t cid = tf->a0;
        if (cid >= N_CAPS) {
                SchedDisablePreemption();
                tf->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                tf->pc += 4;
                SchedEnablePreemption();
                return;
        }
        const Cap cap = cap_node_get_cap(&current->cap_table[cid]);
        SchedDisablePreemption();
        tf->a0 = S3K_ERROR_OK;
        tf->a1 = cap.word0;
        tf->a2 = cap.word1;
        tf->pc += 4;
        SchedEnablePreemption();
}

void syscall_move_cap(TrapFrame *tf) {
        CapNode *cn_src = proc_get_cap_node(current, tf->a0);
        CapNode *cn_dest = proc_get_cap_node(current, tf->a1);
        SchedDisablePreemption();
        tf->pc += 4;
        if (cn_src == NULL || cn_dest == NULL) {
                tf->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
        } else if (cap_node_is_deleted(cn_src)) {
                tf->a0 = S3K_ERROR_CAP_MISSING;
        } else if (!cap_node_is_deleted(cn_dest)) {
                tf->a0 = S3K_ERROR_CAP_COLLISION;
        } else {
                tf->a0 = CapMove(cn_src, cn_dest) ? S3K_ERROR_OK
                                                  : S3K_ERROR_CAP_MISSING;
        }
        SchedEnablePreemption();
}

void syscall_delete_cap(TrapFrame *tf) {
        CapNode *cn = proc_get_cap_node(current, tf->a0);
        if (cn == NULL) {
                SchedDisablePreemption();
                tf->pc += 4;
                tf->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                SchedEnablePreemption();
                return;
        }
        Cap cap = cap_node_get_cap(cn);
        SchedDisablePreemption();
        tf->pc += 4;
        switch (cap_get_type(cap)) {
                case CAP_INVALID:
                        tf->a0 = S3K_ERROR_CAP_MISSING;
                        break;
                case CAP_TIME:
                        tf->a0 = SchedDelete(cap, cn) && CapDelete(cn)
                                     ? S3K_ERROR_OK
                                     : S3K_ERROR_CAP_MISSING;
                        break;
                default:
                        tf->a0 = CapDelete(cn) ? S3K_ERROR_OK
                                               : S3K_ERROR_CAP_MISSING;
                        break;
        }
        SchedEnablePreemption();
}

void syscall_revoke_cap(TrapFrame *tf) {
        CapNode *cn = proc_get_cap_node(current, tf->a0);
        if (cn == NULL) {
                SchedDisablePreemption();
                tf->pc += 4;
                tf->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                SchedEnablePreemption();
                return;
        }
        Cap cap = cap_node_get_cap(cn);
        switch (cap_get_type(cap)) {
                case CAP_TIME:
                        cap_time_set_free(&cap, cap_time_get_begin(cap));
                        CapRevoke(cn);
                        break;
                case CAP_MEMORY:
                        cap_memory_set_free(&cap, cap_memory_get_begin(cap));
                        cap_memory_set_pmp(&cap, false);
                        CapRevoke(cn);
                        break;
                case CAP_SUPERVISOR:
                        cap_supervisor_set_free(&cap,
                                                cap_supervisor_get_begin(cap));
                        CapRevoke(cn);
                        break;
                case CAP_CHANNELS:
                        cap_channels_set_free(&cap,
                                              cap_channels_get_begin(cap));
                        cap_channels_set_ep(&cap, false);
                        CapRevoke(cn);
                        break;
                default:
                        break;
        }
        SchedDisablePreemption();
        /* TODO: Make revocation preemptable */
        switch (cap_get_type(cap)) {
                case CAP_TIME:
                        tf->a0 = SchedRevoke(cap, cn) && CapUpdate(cap, cn)
                                     ? S3K_ERROR_OK
                                     : S3K_ERROR_CAP_MISSING;
                        break;
                case CAP_MEMORY:
                case CAP_SUPERVISOR:
                case CAP_CHANNELS:
                        tf->a0 = CapUpdate(cap, cn) ? S3K_ERROR_OK
                                                    : S3K_ERROR_CAP_MISSING;
                        break;
                case CAP_INVALID:
                        tf->a0 = S3K_ERROR_CAP_MISSING;
                        break;
                default:
                        tf->a0 = S3K_ERROR_NOT_REVOKABLE;
                        break;
        }
        tf->pc += 4;
        SchedEnablePreemption();
}

void syscall_derive_cap(TrapFrame *tf) {
        CapNode *cn_parent = proc_get_cap_node(current, tf->a0);
        CapNode *cn_child = proc_get_cap_node(current, tf->a1);
        if (cn_parent == NULL || cn_child == NULL) {
                SchedDisablePreemption();
                tf->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                tf->pc += 4;
                SchedEnablePreemption();
                return;
        }

        Cap parent = cap_node_get_cap(cn_parent);
        Cap child = (Cap){tf->a2, tf->a3};

        CapType parent_type = cap_get_type(parent);
        CapType child_type = cap_get_type(child);

        if (!cap_can_derive(parent, child)) {
                SchedDisablePreemption();
                tf->a0 = S3K_ERROR_BAD_DERIVATION;
                tf->pc += 4;
                SchedEnablePreemption();
                return;
        }

        if (parent_type == CAP_MEMORY && child_type == CAP_MEMORY) {
                cap_memory_set_free(&parent, cap_memory_get_end(child));
        } else if (parent_type == CAP_MEMORY && child_type == CAP_PMP) {
                cap_memory_set_pmp(&parent, true);
        } else if (parent_type == CAP_TIME && child_type == CAP_TIME) {
                cap_time_set_free(&parent, cap_time_get_end(child));
        } else if (parent_type == CAP_CHANNELS && child_type == CAP_CHANNELS) {
                cap_channels_set_free(&parent, cap_channels_get_end(child));
        } else if (parent_type == CAP_CHANNELS && child_type == CAP_ENDPOINT) {
                cap_channels_set_ep(&parent, true);
        } else if (parent_type == CAP_SUPERVISOR &&
                   child_type == CAP_SUPERVISOR) {
                cap_supervisor_set_free(&parent, cap_supervisor_get_end(child));
        }
        SchedDisablePreemption();
        bool succ = CapUpdate(parent, cn_parent) &&
                    CapInsert(child, cn_child, cn_parent);
        if (parent_type == CAP_TIME && succ) {
                succ = SchedUpdate(parent, child, cn_child);
        }
        tf->a0 = succ ? S3K_ERROR_OK : S3K_ERROR_CAP_MISSING;
        tf->pc += 4;
        SchedEnablePreemption();
}

static void syscall_invoke_endpoint_send(TrapFrame *tf, uint64_t channel) {
        Proc *receiver = channels[channel];
        SchedDisablePreemption();
        if (receiver == NULL) {
                tf->a0 = S3K_ERROR_NO_RECEIVER;
                tf->pc += 4;
                SchedEnablePreemption();
                return;
        }
        if (!__sync_bool_compare_and_swap(&channels[channel], receiver, NULL)) {
                tf->a0 = S3K_ERROR_NO_RECEIVER;
                tf->pc += 4;
                SchedEnablePreemption();
                return;
        }
        while (1) {
                if (__sync_bool_compare_and_swap(&receiver->state, PROC_WAITING,
                                                 PROC_RECEIVING))
                        break;
                if (receiver->state & PROC_SUSPENDED ||
                    receiver->channel != channel) {
                        tf->a0 = S3K_ERROR_NO_RECEIVER;
                        tf->pc += 4;
                        SchedEnablePreemption();
                        return;
                }
        }

        TrapFrame *rtf = receiver->tf;
        receiver->channel = -1;

        /* From where to get capabilities */
        uint64_t cid_src = tf->a1;
        /* From where to set capabilities */
        uint64_t cid_dest = rtf->a1;
        /* Number of capabilities to send */
        uint64_t n = (tf->a2 > rtf->a2) ? rtf->a2 : tf->a2;
        /* Position of last capability to send (excl.) */
        uint64_t cid_last = cid_src + n;
        /* Number of capabilities sent */
        tf->a1 = 0;
        while (cid_src < N_CAPS && cid_src < cid_last && cid_dest < N_CAPS) {
                CapNode *src = &current->cap_table[cid_src];
                CapNode *dest = &receiver->cap_table[cid_dest];
                if (!syscall_interprocess_move(src, dest, receiver->pid)) {
                        /* If moving a capability failed */
                        break;
                }
                cid_src++;
                cid_dest++;
                tf->a1++;
        }
        /* Send successful */
        tf->a0 = S3K_ERROR_OK;
        rtf->a0 = S3K_ERROR_OK;

        /* Number of caps sent */
        rtf->a1 = tf->a1;
        /* Message passing, 256 bits */
        rtf->a2 = tf->a3;
        rtf->a3 = tf->a4;
        rtf->a4 = tf->a5;
        rtf->a5 = tf->a6;
        /* Step forward */
        rtf->pc += 4;
        tf->pc += 4;
        __sync_fetch_and_and(&receiver->state, PROC_SUSPENDED);
        SchedEnablePreemption();
}

void syscall_invoke_endpoint(TrapFrame *tf) {
        CapNode *cn = proc_get_cap_node(current, tf->a0);
        if (cn == NULL) {
                SchedDisablePreemption();
                tf->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                tf->pc += 4;
                SchedEnablePreemption();
                return;
        }
        Cap cap = cap_node_get_cap(cn);

        if (cap_get_type(cap) != CAP_ENDPOINT) {
                SchedDisablePreemption();
                tf->a0 = (cap_get_type(cap) == CAP_INVALID)
                             ? S3K_ERROR_CAP_MISSING
                             : S3K_ERROR_BAD_CAP;
                tf->pc += 4;
                SchedEnablePreemption();
                return;
        }

        uint64_t mode = cap_endpoint_get_mode(cap);
        uint64_t channel = cap_endpoint_get_channel(cap);
        if (mode == 0) { /* Receive */
                current->channel = channel;
                if (__sync_bool_compare_and_swap(&channels[channel], NULL,
                                                 current)) {
                        trap_recv_yield();
                } else {
                        current->channel = -1;
                }
        } else if (mode == 1) { /* Send */
                syscall_invoke_endpoint_send(tf, channel);
        }
}

void syscall_invoke_supervisor_suspend(TrapFrame *tf, Proc *supervisee) {
        SchedDisablePreemption();
        ProcState state =
            __sync_fetch_and_or(&supervisee->state, PROC_SUSPENDED);
        if (state == PROC_WAITING) {
                uint64_t channel = supervisee->channel;
                if (channel != -1) {
                        channels[channel] = NULL;
                }
                __sync_synchronize();
                supervisee->state = PROC_SUSPENDED;
        }
        tf->a0 = (state & PROC_SUSPENDED) == 0;
        tf->pc += 4;
        SchedEnablePreemption();
}

void syscall_invoke_supervisor_resume(TrapFrame *tf, Proc *supervisee) {
        SchedDisablePreemption();
        if (__sync_bool_compare_and_swap(&supervisee->state, PROC_SUSPENDED,
                                         PROC_SUSPENDED_BUSY)) {
                /* Resets the kernel stack */
                supervisee->ksp = supervisee->tf;
                __sync_synchronize();
                supervisee->state = PROC_READY;
                tf->a0 = S3K_ERROR_OK;
        } else {
                tf->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        tf->pc += 4;
        SchedEnablePreemption();
}

void syscall_invoke_supervisor_state(TrapFrame *tf, Proc *supervisee) {
        SchedDisablePreemption();
        tf->a0 = supervisee->state;
        tf->pc += 4;
        SchedEnablePreemption();
}

void syscall_invoke_supervisor_read_reg(TrapFrame *tf, Proc *supervisee) {
        uint64_t reg_nr = tf->a3;
        SchedDisablePreemption();
        tf->pc += 4;
        if (reg_nr >= TF_PROCESS_REGISTERS) {
                tf->a0 = S3K_ERROR_SUPERVISER_REG_NR_OUT_OF_BOUNDS;
        } else if (__sync_bool_compare_and_swap(&supervisee->state,
                                                PROC_SUSPENDED,
                                                PROC_SUSPENDED_BUSY)) {
                tf->a0 = S3K_ERROR_OK;
                tf->a1 = proc_read_register(supervisee, reg_nr);
                __sync_synchronize();
                supervisee->state = PROC_SUSPENDED;
        } else {
                tf->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        SchedEnablePreemption();
}

void syscall_invoke_supervisor_write_reg(TrapFrame *tf, Proc *supervisee) {
        uint64_t reg_nr = tf->a3;
        uint64_t reg_value = tf->a4;
        SchedDisablePreemption();
        tf->pc += 4;
        if (reg_nr >= TF_PROCESS_REGISTERS) {
                tf->a0 = S3K_ERROR_SUPERVISER_REG_NR_OUT_OF_BOUNDS;
        } else if (__sync_bool_compare_and_swap(&supervisee->state,
                                                PROC_SUSPENDED,
                                                PROC_SUSPENDED_BUSY)) {
                tf->a0 = S3K_ERROR_OK;
                tf->a1 = proc_write_register(supervisee, reg_nr, reg_value);
                __sync_synchronize();
                supervisee->state = PROC_SUSPENDED;
        } else {
                tf->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        SchedEnablePreemption();
}

void syscall_invoke_supervisor_give(TrapFrame *tf, Proc *supervisee) {
        uint64_t cid_src = tf->a3;
        uint64_t cid_dest = tf->a4;
        uint64_t cid_last = cid_src + tf->a5;
        SchedDisablePreemption();
        tf->pc += 4;
        if (__sync_bool_compare_and_swap(&supervisee->state, PROC_SUSPENDED,
                                         PROC_SUSPENDED_BUSY)) {
                tf->a0 = S3K_ERROR_OK;
                tf->a1 = 0;
                while (cid_src < N_CAPS && cid_src < cid_last &&
                       cid_dest < N_CAPS) {
                        CapNode *src = &current->cap_table[cid_src];
                        CapNode *dest = &supervisee->cap_table[cid_dest];
                        if (!syscall_interprocess_move(src, dest,
                                                       supervisee->pid))
                                break;
                        cid_src++;
                        cid_dest++;
                        tf->a1++;
                }
                supervisee->state = PROC_SUSPENDED;
        } else {
                tf->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        SchedEnablePreemption();
}

void syscall_invoke_supervisor_take(TrapFrame *tf, Proc *supervisee) {
        uint64_t cid_src = tf->a3;
        uint64_t cid_dest = tf->a4;
        uint64_t cid_last = cid_src + tf->a5;
        SchedDisablePreemption();
        tf->pc += 4;
        if (__sync_bool_compare_and_swap(&supervisee->state, PROC_SUSPENDED,
                                         PROC_SUSPENDED_BUSY)) {
                tf->a1 = 0;
                tf->a0 = S3K_ERROR_OK;
                while (cid_src < N_CAPS && cid_src < cid_last &&
                       cid_dest < N_CAPS) {
                        CapNode *src = &supervisee->cap_table[cid_src];
                        CapNode *dest = &current->cap_table[cid_dest];
                        if (!syscall_interprocess_move(src, dest,
                                                       supervisee->pid))
                                break;
                        cid_src++;
                        cid_dest++;
                        tf->a1++;
                }
                supervisee->state = PROC_SUSPENDED;
        } else {
                tf->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        SchedEnablePreemption();
}

void syscall_invoke_supervisor(TrapFrame *tf) {
        CapNode *cn = proc_get_cap_node(current, tf->a0);
        if (cn == NULL) {
                SchedDisablePreemption();
                tf->pc += 4;
                tf->a0 = S3K_ERROR_CAP_MISSING;
                SchedEnablePreemption();
                return;
        }
        Cap cap = cap_node_get_cap(cn);
        if (cap_get_type(cap) != CAP_SUPERVISOR) {
                SchedDisablePreemption();
                tf->pc += 4;
                tf->a0 = S3K_ERROR_BAD_CAP;
                SchedEnablePreemption();
                return;
        }
        uint64_t pid = tf->a1;
        uint64_t op = tf->a2;
        /* Check free <= pid < end */
        uint64_t pid_free = cap_supervisor_get_free(cap);
        uint64_t pid_end = cap_supervisor_get_end(cap);
        if (pid < pid_free || pid >= pid_end) {
                SchedDisablePreemption();
                tf->pc += 4;
                tf->a0 = S3K_ERROR_SUPERVISER_PID_OUT_OF_BOUNDS;
                SchedEnablePreemption();
                return;
        }

        Proc *supervisee = &processes[pid];
        switch (op) {
                case 0:
                        syscall_invoke_supervisor_suspend(tf, supervisee);
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
                        SchedDisablePreemption();
                        tf->pc += 4;
                        tf->a0 = S3K_ERROR_SUPERVISER_BAD_OP;
                        SchedEnablePreemption();
                        break;
        }
}

