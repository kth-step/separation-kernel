// See LICENSE file for copyright and license details.
#pragma once

#include "proc.h"
#include "proc_state.h"
void syscall_channels_revoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_receiver_revoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_server_revoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));

void syscall_channels_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
    __attribute__((noreturn));
void syscall_receiver_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
    __attribute__((noreturn));
void syscall_server_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
    __attribute__((noreturn));

void syscall_receiver_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_sender_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_server_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_client_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
