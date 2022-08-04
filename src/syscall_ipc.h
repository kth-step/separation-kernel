#pragma once

#include "proc.h"
#include "proc_state.h"

void syscall_receiver_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_sender_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_server_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
void syscall_client_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
