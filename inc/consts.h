// See LICENSE file for copyright and license details.
#pragma once

typedef enum proc_state proc_state_t;
typedef enum s3k_error s3k_error_t;
typedef enum s3k_call s3k_call_t;
typedef enum s3k_call_sup s3k_call_sup_t;

enum proc_state {
    PROC_STATE_READY,
    PROC_STATE_RUNNING,
    PROC_STATE_WAITING,
    PROC_STATE_RECEIVING,
    PROC_STATE_SUSPENDED,
    PROC_STATE_RUNNING_THEN_SUSPEND,
    PROC_STATE_SUSPENDED_BUSY,
    PROC_STATE_RECEIVING_THEN_SUSPEND
};

enum s3k_error {
    ERROR_OK, /* No error */
    ERROR_EMPTY,
    ERROR_FAILED,
    ERROR_PREEMPTED,
    ERROR_INTERRUPTED,
    ERROR_COLLISION,
    ERROR_NO_RECEIVER,
    ERROR_ILLEGAL_DERIVATION,
    ERROR_INVALID_SUPERVISEE,
    ERROR_SUPERVISEE_BUSY,
    ERROR_UNIMPLEMENTED = -1
};

enum s3k_call {
    ECALL_UNIMPLEMENTED,
    ECALL_READ_CAP,
    ECALL_MOVE_CAP,
    ECALL_DELETE_CAP,
    ECALL_REVOKE_CAP,
    ECALL_DERIVE_CAP,
    ECALL_INVOKE_CAP,
    ECALL_GET_PID,
    ECALL_READ_REG,
    ECALL_WRITE_REG,
    ECALL_YIELD,
    NUM_OF_SYSNR
};

enum s3k_call_sup {
    ECALL_SUP_SUSPEND,
    ECALL_SUP_RESUME,
    ECALL_SUP_GET_STATE,
    ECALL_SUP_READ_REG,
    ECALL_SUP_WRITE_REG,
    ECALL_SUP_READ_CAP,
    ECALL_SUP_GIVE_CAP,
    ECALL_SUP_TAKE_CAP,
};
