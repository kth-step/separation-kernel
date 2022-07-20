#include "syscall_ipc.h"
#include "preemption.h"
#include "s3k_consts.h"

static inline uint64_t channels_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap);
static inline uint64_t channels_revoke_cap(cap_node_t* cn, cap_t cap);
static inline uint64_t receiver_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap);
static inline uint64_t receiver_revoke_cap(cap_node_t* cn, cap_t cap);

uint64_t channels_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
    kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);

    if (!cap_node_is_deleted(newcn))
        return S3K_COLLISION;

    if (!cap_can_derive(cap, newcap))
        return S3K_ILLEGAL_DERIVATION;

    kassert(cap_get_type(newcap) == CAP_TYPE_CHANNELS || cap_get_type(newcap) == CAP_TYPE_RECEIVER);

    if (cap_get_type(newcap) == CAP_TYPE_CHANNELS)
        cap_channels_set_free(&cap, cap_channels_get_end(newcap));

    if (cap_get_type(newcap) == CAP_TYPE_RECEIVER)
        cap_channels_set_free(&cap, cap_receiver_get_channel(newcap) + 1);

    if (cap_node_update(cap, cn) && cap_node_insert(newcap, newcn, cn))
        return S3K_OK;

    return S3K_ERROR;
}

uint64_t channels_revoke_cap(cap_node_t* cn, cap_t cap)
{
    kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);

    preemption_enable();
    cap_node_revoke(cn);
    preemption_disable();

    cap_channels_set_free(&cap, cap_channels_get_begin(cap));

    if (cap_node_update(cap, cn))
        return S3K_OK;

    return S3K_ERROR;
}

uint64_t receiver_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
{
    kassert(cap_get_type(cap) == CAP_TYPE_RECEIVER);

    if (!cap_node_is_deleted(newcn))
        return S3K_COLLISION;

    if (!cap_can_derive(cap, newcap))
        return S3K_ILLEGAL_DERIVATION;

    if (cap_node_insert(newcap, newcn, cn))
        return S3K_OK;

    return S3K_ERROR;
}

uint64_t receiver_revoke_cap(cap_node_t* cn, cap_t cap)
{
    kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);

    preemption_enable();
    cap_node_revoke(cn);
    preemption_disable();

    return S3K_OK;
}

void syscall_handle_channels(registers_t* regs, cap_node_t* cn, cap_t cap)
{
    kassert(cap_get_type(cap) == CAP_TYPE_CHANNELS);
    preemption_disable();
    switch (regs->a7) {

    case S3K_SYSNR_READ_CAP:
        regs->a0 = S3K_OK;
        regs->a1 = cap.word0;
        regs->a2 = cap.word1;
        break;

    case S3K_SYSNR_MOVE_CAP:
        regs->a0 = cap_node_move(cn, proc_get_cap_node(current, regs->a1)) ? S3K_OK : S3K_ERROR;
        break;

    case S3K_SYSNR_DELETE_CAP:
        regs->a0 = cap_node_delete(cn) ? S3K_OK : S3K_ERROR;
        break;

    case S3K_SYSNR_REVOKE_CAP:
        preemption_enable();
        cap_node_revoke(cn);
        preemption_disable();
        regs->a0 = S3K_OK;
        break;

    case S3K_SYSNR_DERIVE_CAP: {
        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
        cap_t newcap = (cap_t) { regs->a2, regs->a3 };
        regs->a0 = channels_derive_cap(cn, cap, newcn, newcap);
    } break;

    default:
        regs->a0 = S3K_UNIMPLEMENTED;
        break;
    }
    regs->pc += 4;
    preemption_enable();
}

void syscall_handle_receiver(registers_t* regs, cap_node_t* cn, cap_t cap)
{
    kassert(cap_get_type(cap) == CAP_TYPE_RECEIVER);
    preemption_disable();
    switch (regs->a7) {
    case S3K_SYSNR_READ_CAP:
        regs->a0 = S3K_OK;
        regs->a1 = cap.word0;
        regs->a2 = cap.word1;
        break;

    case S3K_SYSNR_MOVE_CAP:
        regs->a0 = cap_node_move(cn, proc_get_cap_node(current, regs->a1)) ? S3K_OK : S3K_ERROR;
        break;

    case S3K_SYSNR_DELETE_CAP:
        regs->a0 = cap_node_delete(cn) ? S3K_OK : S3K_ERROR;
        break;

    case S3K_SYSNR_REVOKE_CAP:
        preemption_enable();
        cap_node_revoke(cn);
        preemption_disable();
        regs->a0 = S3K_OK;
        break;

    case S3K_SYSNR_DERIVE_CAP: {
        cap_node_t* newcn = proc_get_cap_node(current, regs->a1);
        cap_t newcap = (cap_t) { regs->a2, regs->a3 };
        regs->a0 = receiver_derive_cap(cn, cap, newcn, newcap);
    } break;

    default:
        regs->a0 = S3K_UNIMPLEMENTED;
        break;
    }
    regs->pc += 4;
    preemption_enable();
}

void syscall_handle_sender(registers_t* regs, cap_node_t* cn, cap_t cap)
{
    kassert(cap_get_type(cap) == CAP_TYPE_SENDER);
    preemption_disable();
    switch (regs->a7) {

    case S3K_SYSNR_READ_CAP:
        regs->a0 = S3K_OK;
        regs->a1 = cap.word0;
        regs->a2 = cap.word1;
        break;

    case S3K_SYSNR_MOVE_CAP:
        regs->a0 = cap_node_move(cn, proc_get_cap_node(current, regs->a1)) ? S3K_OK : S3K_ERROR;
        break;

    case S3K_SYSNR_DELETE_CAP:
        regs->a0 = cap_node_delete(cn) ? S3K_OK : S3K_ERROR;
        break;

    default:
        regs->a0 = S3K_UNIMPLEMENTED;
        break;
    }
    regs->pc += 4;
    preemption_enable();
}
