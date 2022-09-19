// See LICENSE file for copyright and license details.
#pragma once

/* No error */
#define S3K_OK 0
/* System call preempted */
#define S3K_PREEMPTED 1
/* Capability missing */
#define S3K_EMPTY 2
/* In move/derive, target destination is occupied */
#define S3K_COLLISION 3
/* Capability can not be derived */
#define S3K_ILLEGAL_DERIVATION 4
/* Invalid capability for operation */
#define S3K_BAD_CAP 5
/* Endpoint send operation aborted */
#define S3K_SEND_ABORTED 6
/* Endpoint send operation has no receiver */
#define S3K_NO_RECEIVER 7
#define S3K_INTERRUPTED 8

/* Some other thread has locked the supervisee. */
#define S3K_SUPERVISEE_BUSY 9
#define S3K_INVALID_SUPERVISEE 10
#define S3K_ALREADY_SUSPENDED 11

#define S3K_FAILED 12
/* Unimplemented operation for capability */
#define S3K_UNIMPLEMENTED 0xFFFFFFFFFFFFFFFFull

#define S3K_SYSNR_READ_CAP 1
#define S3K_SYSNR_MOVE_CAP 2
#define S3K_SYSNR_DELETE_CAP 3
#define S3K_SYSNR_REVOKE_CAP 4
#define S3K_SYSNR_DERIVE_CAP 5
#define S3K_SYSNR_INVOKE_CAP 6
#define S3K_SYSNR_GET_PID 7
#define S3K_SYSNR_WRITE_REG 8
#define S3K_SYSNR_READ_REG 9
#define S3K_SYSNR_YIELD 10
#define NUM_OF_SYSNR 11

#define S3K_SYSNR_INVOKE_SUPERVISOR_SUSPEND 0
#define S3K_SYSNR_INVOKE_SUPERVISOR_RESUME 1
#define S3K_SYSNR_INVOKE_SUPERVISOR_GET_STATE 2
#define S3K_SYSNR_INVOKE_SUPERVISOR_READ_REG 3
#define S3K_SYSNR_INVOKE_SUPERVISOR_WRITE_REG 4
#define S3K_SYSNR_INVOKE_SUPERVISOR_READ_CAP 5
#define S3K_SYSNR_INVOKE_SUPERVISOR_GIVE_CAP 6
#define S3K_SYSNR_INVOKE_SUPERVISOR_TAKE_CAP 7
