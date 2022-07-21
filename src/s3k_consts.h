#pragma once

typedef enum s3k_error S3KError;
typedef enum s3k_sysnr S3KSysnr;
typedef enum s3k_sysnr_nocap S3KSysnrNoCap;

enum s3k_error {
        /* No error */
        S3K_OK,
        /* */
        S3K_ERROR,
        /* Capability missing */
        S3K_EMPTY,
        /* In move/derive, target destination is occupied */
        S3K_COLLISION,
        /* Capability index out of bounds */
        S3K_INDEX_OUT_OF_BOUNDS,
        /* Capability can not be derived */
        S3K_ILLEGAL_DERIVATION,
        /* Wrong capability for invocation */
        S3K_BAD_CAP,
        /* Capability has no children, can not revoke */
        S3K_CAP_NOT_REVOKEABLE,
        /* PID in superviser call is out of bounds */
        S3K_PID_NOT_IN_RANGE,
        /* */
        S3K_REG_NOT_IN_RANGE,
        /* */
        S3K_SUPERVISEE_SUSPENDED,
        /* Some other thread has locked the supervisee. */
        S3K_SUPERVISEE_BUSY,
        /* Endpoint send operation aborted */
        S3K_SEND_ABORTED,
        /* Endpoint send operation has no receiver */
        S3K_NO_RECEIVER,
        /* */
        S3K_UNIMPLEMENTED = 0xFF
};

enum s3k_sysnr {
        S3K_SYSNR_GET_PID,
        S3K_SYSNR_WRITE_REGISTER,
        S3K_SYSNR_READ_REGISTER,
        S3K_SYSNR_YIELD,
        S3K_SYSNR_READ_CAP,
        S3K_SYSNR_MOVE_CAP,
        S3K_SYSNR_DELETE_CAP,
        S3K_SYSNR_REVOKE_CAP,
        S3K_SYSNR_DERIVE_CAP,
        S3K_SYSNR_IPC_RECEIVE,
        S3K_SYSNR_IPC_SEND,
        S3K_SYSNR_SUPERVISOR_SUSPEND,
        S3K_SYSNR_SUPERVISOR_RESUME,
        S3K_SYSNR_SUPERVISOR_GET_STATE,
        S3K_SYSNR_SUPERVISOR_READ_REG,
        S3K_SYSNR_SUPERVISOR_WRITE_REG,
        S3K_SYSNR_SUPERVISOR_GIVE_CAP,
        S3K_SYSNR_SUPERVISOR_TAKE_CAP
};
