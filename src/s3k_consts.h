#pragma once

typedef enum s3k_error {
        /* No error */
        S3K_OK,
        /* Unspecified error */
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
} s3k_error_t;

typedef enum s3k_sysnr {
        S3K_SYSNR_READ_CAP,
        S3K_SYSNR_MOVE_CAP,
        S3K_SYSNR_DELETE_CAP,
        S3K_SYSNR_REVOKE_CAP,
        S3K_SYSNR_DERIVE_CAP,
        S3K_SYSNR_INVOKE_CAP,
        S3K_SYSNR_GET_PID,
        S3K_SYSNR_WRITE_REGISTER,
        S3K_SYSNR_READ_REGISTER,
        S3K_SYSNR_YIELD,
        NUM_OF_SYSNR
} s3k_sysnr_t;
