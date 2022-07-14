// See LICENSE file for copyright and license details.

#include "cap_utils.h"
#include "csr.h"
#include "preemption.h"
#include "s3k_consts.h"
#include "sched.h"
#include "trap.h"
#include "types.h"
#include "utils.h"

static void syscall_no_cap(struct registers *regs);
static void syscall_read_cap(struct registers *regs);
static void syscall_move_cap(struct registers *regs);
static void syscall_delete_cap(struct registers *regs);
static void syscall_revoke_cap(struct registers *regs);
static void syscall_derive_cap(struct registers *regs);

/* Invoke functions */
static void syscall_invoke_endpoint(struct registers *regs);
static void syscall_invoke_supervisor(struct registers *regs);

static bool syscall_interprocess_move(CapNode *src, CapNode *dest,
                                      uint64_t pid);

static void (*const syscall_handler_array[])(struct registers *) = {
    syscall_no_cap,          syscall_read_cap,         syscall_move_cap,
    syscall_delete_cap,      syscall_revoke_cap,       syscall_derive_cap,
    syscall_invoke_endpoint, syscall_invoke_supervisor};

struct proc *volatile channels[N_CHANNELS];

void syscall_handler(struct registers *regs, uint64_t mcause, uint64_t mtval) {
        uint64_t syscall_number = regs->t0;
        if (syscall_number < ARRAY_SIZE(syscall_handler_array)) {
                syscall_handler_array[syscall_number](regs);
        } else {
                exception_handler(regs, mcause, mtval);
        }
}

void syscall_no_cap(struct registers *regs) {
        uint64_t op_number = regs->a0;
        switch (op_number) {
                case S3K_SYSNR_NOCAP_GET_PID:
                        preemption_disable();
                        /* Get the struct process ID */
                        regs->a0 = S3K_ERROR_OK;
                        regs->a1 = current->pid;
                        regs->pc += 4;
                        preemption_enable();
                        break;
                case S3K_SYSNR_NOCAP_READ_REGISTER:
                        preemption_disable();
                        regs->a0 = S3K_ERROR_OK;
                        regs->a1 = proc_read_register(current, regs->a1);
                        regs->pc += 4;
                        preemption_enable();
                        break;
                case S3K_SYSNR_NOCAP_WRITE_REGISTER:
                        preemption_disable();
                        regs->a0 = S3K_ERROR_OK;
                        regs->a1 =
                            proc_write_register(current, regs->a1, regs->a2);
                        regs->pc += 4;
                        preemption_enable();
                        break;
                default:
                        preemption_disable();
                        regs->a0 = S3K_ERROR_NOCAP_BAD_OP;
                        regs->pc += 4;
                        preemption_enable();
                        break;
        }
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

void syscall_read_cap(struct registers *regs) {
        CapNode *cn = proc_get_cap_node(current, regs->a0);
        if (cn == NULL) {
                preemption_disable();
                regs->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                regs->pc += 4;
                preemption_enable();
        } else {
                Cap cap = cap_node_get_cap(cn);
                preemption_disable();
                regs->a0 = S3K_ERROR_OK;
                regs->a1 = cap.word0;
                regs->a2 = cap.word1;
                regs->pc += 4;
                preemption_enable();
        }
}

void syscall_move_cap(struct registers *regs) {
        CapNode *cn_src = proc_get_cap_node(current, regs->a0);
        CapNode *cn_dest = proc_get_cap_node(current, regs->a1);
        preemption_disable();
        regs->pc += 4;
        if (cn_src == NULL || cn_dest == NULL) {
                regs->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
        } else if (cap_node_is_deleted(cn_src)) {
                regs->a0 = S3K_ERROR_CAP_MISSING;
        } else if (!cap_node_is_deleted(cn_dest)) {
                regs->a0 = S3K_ERROR_CAP_COLLISION;
        } else {
                regs->a0 = CapMove(cn_src, cn_dest) ? S3K_ERROR_OK
                                                    : S3K_ERROR_CAP_MISSING;
        }
        preemption_enable();
}

void syscall_delete_cap(struct registers *regs) {
        CapNode *cn = proc_get_cap_node(current, regs->a0);
        if (cn == NULL) {
                regs->pc += 4;
                regs->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
        } else {
                Cap cap = cap_node_get_cap(cn);
                regs->pc += 4;
                switch (cap_get_type(cap)) {
                        case CAP_INVALID:
                                regs->a0 = S3K_ERROR_CAP_MISSING;
                                break;
                        case CAP_TIME:
                                regs->a0 = SchedDelete(cap, cn) && CapDelete(cn)
                                               ? S3K_ERROR_OK
                                               : S3K_ERROR_CAP_MISSING;
                                break;
                        default:
                                regs->a0 = CapDelete(cn)
                                               ? S3K_ERROR_OK
                                               : S3K_ERROR_CAP_MISSING;
                                break;
                }
        }
}

void syscall_revoke_cap(struct registers *regs) {
        CapNode *cn = proc_get_cap_node(current, regs->a0);
        if (cn == NULL) {
                preemption_disable();
                regs->pc += 4;
                regs->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                preemption_enable();
                return;
        }
        Cap cap = cap_node_get_cap(cn);
        CapType type = cap_get_type(cap);
        switch (type) {
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
                        preemption_disable();
                        regs->pc += 4;
                        regs->a0 = (type == CAP_INVALID)
                                       ? S3K_ERROR_CAP_MISSING
                                       : S3K_ERROR_NOT_REVOKABLE;
                        preemption_enable();
                        return;
        }
        preemption_disable();
        if (type == CAP_TIME) {
                regs->a0 = SchedRevoke(cap, cn) && CapUpdate(cap, cn)
                               ? S3K_ERROR_OK
                               : S3K_ERROR_CAP_MISSING;
        } else {
                regs->a0 =
                    CapUpdate(cap, cn) ? S3K_ERROR_OK : S3K_ERROR_CAP_MISSING;
        }
        regs->pc += 4;
        preemption_enable();
}

void syscall_derive_cap(struct registers *regs) {
        CapNode *cn_parent = proc_get_cap_node(current, regs->a0);
        CapNode *cn_child = proc_get_cap_node(current, regs->a1);
        if (cn_parent == NULL || cn_child == NULL) {
                preemption_disable();
                regs->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                regs->pc += 4;
                preemption_enable();
                return;
        }

        Cap parent = cap_node_get_cap(cn_parent);
        Cap child = (Cap){regs->a2, regs->a3};

        CapType parent_type = cap_get_type(parent);
        CapType child_type = cap_get_type(child);

        if (!cap_can_derive(parent, child)) {
                preemption_disable();
                regs->a0 = S3K_ERROR_BAD_DERIVATION;
                regs->pc += 4;
                preemption_enable();
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
        preemption_disable();
        bool succ = CapUpdate(parent, cn_parent) &&
                    CapInsert(child, cn_child, cn_parent);
        if (parent_type == CAP_TIME && succ) {
                succ = SchedUpdate(parent, child, cn_child);
        }
        regs->a0 = succ ? S3K_ERROR_OK : S3K_ERROR_CAP_MISSING;
        regs->pc += 4;
        preemption_enable();
}

static void syscall_invoke_endpoint_send(struct registers *regs,
                                         uint64_t channel) {
        struct proc *receiver = channels[channel];
        preemption_disable();
        if (receiver == NULL) {
                regs->a0 = S3K_ERROR_NO_RECEIVER;
                regs->pc += 4;
                preemption_enable();
                return;
        }
        if (!__sync_bool_compare_and_swap(&channels[channel], receiver, NULL)) {
                regs->a0 = S3K_ERROR_NO_RECEIVER;
                regs->pc += 4;
                preemption_enable();
                return;
        }
        while (1) {
                if (__sync_bool_compare_and_swap(&receiver->state, PS_WAITING,
                                                 PS_RECEIVING))
                        break;
                if (receiver->state & PS_SUSPENDED ||
                    receiver->channel != channel) {
                        regs->a0 = S3K_ERROR_NO_RECEIVER;
                        regs->pc += 4;
                        preemption_enable();
                        return;
                }
        }

        struct registers *rregs = &receiver->regs;
        receiver->channel = -1;

        /* From where to get capabilities */
        uint64_t cid_src = regs->a1;
        /* From where to set capabilities */
        uint64_t cid_dest = rregs->a1;
        /* Number of capabilities to send */
        uint64_t n = (regs->a2 > rregs->a2) ? rregs->a2 : regs->a2;
        /* Position of last capability to send (excl.) */
        uint64_t cid_last = cid_src + n;
        /* Number of capabilities sent */
        regs->a1 = 0;
        while (cid_src < N_CAPS && cid_src < cid_last && cid_dest < N_CAPS) {
                CapNode *src = &current->cap_table[cid_src];
                CapNode *dest = &receiver->cap_table[cid_dest];
                if (!syscall_interprocess_move(src, dest, receiver->pid)) {
                        /* If moving a capability failed */
                        break;
                }
                cid_src++;
                cid_dest++;
                regs->a1++;
        }
        /* Send successful */
        regs->a0 = S3K_ERROR_OK;
        rregs->a0 = S3K_ERROR_OK;

        /* Number of caps sent */
        rregs->a1 = regs->a1;
        /* Message passing, 256 bits */
        rregs->a2 = regs->a3;
        rregs->a3 = regs->a4;
        rregs->a4 = regs->a5;
        rregs->a5 = regs->a6;
        /* Step forward */
        rregs->pc += 4;
        regs->pc += 4;
        __sync_fetch_and_and(&receiver->state, PS_SUSPENDED);
        preemption_enable();
}

void syscall_invoke_endpoint(struct registers *regs) {
        CapNode *cn = proc_get_cap_node(current, regs->a0);
        if (cn == NULL) {
                preemption_disable();
                regs->a0 = S3K_ERROR_INDEX_OUT_OF_BOUNDS;
                regs->pc += 4;
                preemption_enable();
                return;
        }
        Cap cap = cap_node_get_cap(cn);

        if (cap_get_type(cap) != CAP_ENDPOINT) {
                preemption_disable();
                regs->a0 = (cap_get_type(cap) == CAP_INVALID)
                               ? S3K_ERROR_CAP_MISSING
                               : S3K_ERROR_BAD_CAP;
                regs->pc += 4;
                preemption_enable();
                return;
        }

        uint64_t mode = cap_endpoint_get_mode(cap);
        uint64_t channel = cap_endpoint_get_channel(cap);
        if (mode == 0) { /* Receive */
                current->channel = channel;
                if (__sync_bool_compare_and_swap(&channels[channel], NULL,
                                                 current)) {
                } else {
                        current->channel = -1;
                }
        } else if (mode == 1) { /* Send */
                syscall_invoke_endpoint_send(regs, channel);
        }
}

void syscall_invoke_supervisor_suspend(struct registers *regs,
                                       struct proc *supervisee) {
        preemption_disable();
        enum proc_state state =
            __sync_fetch_and_or(&supervisee->state, PS_SUSPENDED);
        if (state == PS_WAITING) {
                uint64_t channel = supervisee->channel;
                if (channel != -1) {
                        channels[channel] = NULL;
                }
                __sync_synchronize();
                supervisee->state = PS_SUSPENDED;
        }
        regs->a0 = (state & PS_SUSPENDED) == 0;
        regs->pc += 4;
        preemption_enable();
}

void syscall_invoke_supervisor_resume(struct registers *regs,
                                      struct proc *supervisee) {
        preemption_disable();
        if (__sync_bool_compare_and_swap(&supervisee->state, PS_SUSPENDED,
                                         PS_SUSPENDED_BUSY)) {
                __sync_synchronize();
                supervisee->state = PS_READY;
                regs->a0 = S3K_ERROR_OK;
        } else {
                regs->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        regs->pc += 4;
        preemption_enable();
}

void syscall_invoke_supervisor_state(struct registers *regs,
                                     struct proc *supervisee) {
        preemption_disable();
        regs->a0 = supervisee->state;
        regs->pc += 4;
        preemption_enable();
}

void syscall_invoke_supervisor_read_reg(struct registers *regs,
                                        struct proc *supervisee) {
        uint64_t reg_nr = regs->a3;
        preemption_disable();
        regs->pc += 4;
        if (reg_nr >= N_REGISTERS) {
                regs->a0 = S3K_ERROR_SUPERVISER_REG_NR_OUT_OF_BOUNDS;
        } else if (__sync_bool_compare_and_swap(
                       &supervisee->state, PS_SUSPENDED, PS_SUSPENDED_BUSY)) {
                regs->a0 = S3K_ERROR_OK;
                regs->a1 = proc_read_register(supervisee, reg_nr);
                __sync_synchronize();
                supervisee->state = PS_SUSPENDED;
        } else {
                regs->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        preemption_enable();
}

void syscall_invoke_supervisor_write_reg(struct registers *regs,
                                         struct proc *supervisee) {
        uint64_t reg_nr = regs->a3;
        uint64_t reg_value = regs->a4;
        preemption_disable();
        regs->pc += 4;
        if (reg_nr >= N_REGISTERS) {
                regs->a0 = S3K_ERROR_SUPERVISER_REG_NR_OUT_OF_BOUNDS;
        } else if (__sync_bool_compare_and_swap(
                       &supervisee->state, PS_SUSPENDED, PS_SUSPENDED_BUSY)) {
                regs->a0 = S3K_ERROR_OK;
                regs->a1 = proc_write_register(supervisee, reg_nr, reg_value);
                __sync_synchronize();
                supervisee->state = PS_SUSPENDED;
        } else {
                regs->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        preemption_enable();
}

void syscall_invoke_supervisor_give(struct registers *regs,
                                    struct proc *supervisee) {
        uint64_t cid_src = regs->a3;
        uint64_t cid_dest = regs->a4;
        uint64_t cid_last = cid_src + regs->a5;
        preemption_disable();
        regs->pc += 4;
        if (__sync_bool_compare_and_swap(&supervisee->state, PS_SUSPENDED,
                                         PS_SUSPENDED_BUSY)) {
                regs->a0 = S3K_ERROR_OK;
                regs->a1 = 0;
                while (cid_src < N_CAPS && cid_src < cid_last &&
                       cid_dest < N_CAPS) {
                        CapNode *src = &current->cap_table[cid_src];
                        CapNode *dest = &supervisee->cap_table[cid_dest];
                        if (!syscall_interprocess_move(src, dest,
                                                       supervisee->pid))
                                break;
                        cid_src++;
                        cid_dest++;
                        regs->a1++;
                }
                supervisee->state = PS_SUSPENDED;
        } else {
                regs->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        preemption_enable();
}

void syscall_invoke_supervisor_take(struct registers *regs,
                                    struct proc *supervisee) {
        uint64_t cid_src = regs->a3;
        uint64_t cid_dest = regs->a4;
        uint64_t cid_last = cid_src + regs->a5;
        preemption_disable();
        if (__sync_bool_compare_and_swap(&supervisee->state, PS_SUSPENDED,
                                         PS_SUSPENDED_BUSY)) {
                regs->pc += 4;
                regs->a1 = 0;
                regs->a0 = S3K_ERROR_OK;
                while (cid_src < N_CAPS && cid_src < cid_last &&
                       cid_dest < N_CAPS) {
                        CapNode *src = &supervisee->cap_table[cid_src];
                        CapNode *dest = &current->cap_table[cid_dest];
                        if (!syscall_interprocess_move(src, dest,
                                                       supervisee->pid))
                                break;
                        cid_src++;
                        cid_dest++;
                        regs->a1++;
                }
                supervisee->state = PS_SUSPENDED;
        } else {
                regs->pc += 4;
                regs->a0 = S3K_ERROR_SUPERVISEE_BUSY;
        }
        preemption_enable();
}

void syscall_invoke_supervisor(struct registers *regs) {
        CapNode *cn = proc_get_cap_node(current, regs->a0);
        if (cn == NULL) {
                preemption_disable();
                regs->pc += 4;
                regs->a0 = S3K_ERROR_CAP_MISSING;
                preemption_enable();
                return;
        }
        Cap cap = cap_node_get_cap(cn);
        if (cap_get_type(cap) != CAP_SUPERVISOR) {
                preemption_disable();
                regs->pc += 4;
                regs->a0 = S3K_ERROR_BAD_CAP;
                preemption_enable();
                return;
        }
        uint64_t pid = regs->a1;
        uint64_t op = regs->a2;
        /* Check free <= pid < end */
        uint64_t pid_free = cap_supervisor_get_free(cap);
        uint64_t pid_end = cap_supervisor_get_end(cap);
        if (pid < pid_free || pid >= pid_end) {
                preemption_disable();
                regs->pc += 4;
                regs->a0 = S3K_ERROR_SUPERVISER_PID_OUT_OF_BOUNDS;
                preemption_enable();
                return;
        }

        struct proc *supervisee = &processes[pid];
        switch (op) {
                case 0:
                        syscall_invoke_supervisor_suspend(regs, supervisee);
                        break;
                case 1:
                        syscall_invoke_supervisor_resume(regs, supervisee);
                        break;
                case 2:
                        syscall_invoke_supervisor_state(regs, supervisee);
                        break;
                case 3:
                        syscall_invoke_supervisor_read_reg(regs, supervisee);
                        break;
                case 4:
                        syscall_invoke_supervisor_write_reg(regs, supervisee);
                        break;
                case 5:
                        syscall_invoke_supervisor_give(regs, supervisee);
                        break;
                case 6:
                        syscall_invoke_supervisor_take(regs, supervisee);
                        break;
                default:
                        preemption_disable();
                        regs->pc += 4;
                        regs->a0 = S3K_ERROR_SUPERVISER_BAD_OP;
                        preemption_enable();
                        break;
        }
}

