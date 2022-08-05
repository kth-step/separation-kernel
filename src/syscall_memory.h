// See LICENSE file for copyright and license details.
#pragma once

#include "cap_node.h"

void syscall_memory_revoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_memory_derive_cap(cap_node_t* cn, cap_t cap, cap_node_t* newcn, cap_t newcap)
    __attribute__((noreturn));
