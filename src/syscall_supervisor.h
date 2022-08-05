// See LICENSE file for copyright and license details.
#pragma once

#include "proc.h"

void syscall_supervisor_revoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_supervisor_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
    __attribute__((noreturn));
void syscall_supervisor_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
