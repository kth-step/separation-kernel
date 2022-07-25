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
        /* Capability can not be derived */
        S3K_ILLEGAL_DERIVATION,
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
        S3K_SYSNR_GET_PID = -1,
        S3K_SYSNR_WRITE_REGISTER = -2,
        S3K_SYSNR_READ_REGISTER = -3,
        S3K_SYSNR_YIELD = -4,

        S3K_SYSNR_READ_CAP = 0,
        S3K_SYSNR_MOVE_CAP,
        S3K_SYSNR_DELETE_CAP,
        S3K_SYSNR_REVOKE_CAP,
        S3K_SYSNR_DERIVE_CAP,
        S3K_SYSNR_INVOKE_CAP,
        NUM_OF_CAP_SYSNR
};
