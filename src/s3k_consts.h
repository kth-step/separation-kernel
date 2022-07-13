#pragma once

typedef enum s3k_error S3KError;
typedef enum s3k_sysnr S3KSysnr;
typedef enum s3k_sysnr_nocap S3KSysnrNoCap;

enum s3k_error {
        /* No error */
        S3K_ERROR_OK,
        /* Unspecified error */
        S3K_ERROR_FAILED,
        /* Capability to use with system call missing */
        S3K_ERROR_CAP_MISSING,
        /* In move/derive, target destination is occupied */
        S3K_ERROR_CAP_COLLISION,
        /* Capability index out of bounds */
        S3K_ERROR_INDEX_OUT_OF_BOUNDS,
        /* Capability can not be derived */
        S3K_ERROR_BAD_DERIVATION,
        /* Wrong capability for invocation */
        S3K_ERROR_BAD_CAP,
        /* Capability has no children, can not revoke */
        S3K_ERROR_NOT_REVOKABLE,
        /* Op number for NOCAP operation is bad */
        S3K_ERROR_NOCAP_BAD_OP,
        /* Op number for SUPERVISER operation is bad */
        S3K_ERROR_SUPERVISER_BAD_OP,
        /* PID in superviser call is out of bounds */
        S3K_ERROR_SUPERVISER_PID_OUT_OF_BOUNDS,
        /* */
        S3K_ERROR_SUPERVISER_REG_NR_OUT_OF_BOUNDS,
        /* Some other thread has locked the supervisee. */
        S3K_ERROR_SUPERVISEE_BUSY,
        /* Endpoint send operation aborted */
        S3K_ERROR_SEND_ABORTED,
        /* Endpoint send operation has no receiver */
        S3K_ERROR_NO_RECEIVER,
};

enum s3k_syscall {
        S3K_SYSNR_NOCAP,
        S3K_SYSNR_READ_CAP,
        S3K_SYSNR_MOVE_CAP,
        S3K_SYSNR_DELETE_CAP,
        S3K_SYSNR_REVOKE_CAP,
        S3K_SYSNR_DERIVE_CAP,
        S3K_SYSNR_INVOKE_ENDPOINT_CAP,
        S3K_SYSNR_INVOKE_SUPERVISOR_CAP,
};

enum s3k_syscall_nocap {
        S3K_SYSNR_NOCAP_GET_PID,
        S3K_SYSNR_NOCAP_WRITE_REGISTER,
        S3K_SYSNR_NOCAP_READ_REGISTER,
        S3K_SYSNR_NOCAP_YIELD
};
