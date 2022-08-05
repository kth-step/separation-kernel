// See LICENSE file for copyright and license details.
#pragma once

/* No error */
#define S3K_OK 0
/* Unspecified error */
#define S3K_ERROR 1
/* Capability missing */
#define S3K_EMPTY 2
/* In move/derive, target destination is occupied */
#define S3K_COLLISION 3
/* Capability can not be derived */
#define S3K_ILLEGAL_DERIVATION 4
/* Some other thread has locked the supervisee. */
#define S3K_SUPERVISEE_BUSY 5
/* Endpoint send operation aborted */
#define S3K_SEND_ABORTED 6
/* Endpoint send operation has no receiver */
#define S3K_NO_RECEIVER 7
/* Unimplemented operation for capability */
#define S3K_UNIMPLEMENTED 0xFF

#define S3K_SYSNR_GET_PID 0
#define S3K_SYSNR_WRITE_REGISTER 1
#define S3K_SYSNR_READ_REGISTER 2
#define S3K_SYSNR_YIELD 3
#define S3K_SYSNR_READ_CAP 4
#define S3K_SYSNR_MOVE_CAP 5
#define S3K_SYSNR_DELETE_CAP 6
#define S3K_SYSNR_REVOKE_CAP 7
#define S3K_SYSNR_DERIVE_CAP 8
#define S3K_SYSNR_INVOKE_CAP 9
#define NUM_OF_SYSNR 10
