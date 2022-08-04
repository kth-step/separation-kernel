#pragma once

#include "proc.h"

void syscall_supervisor_invoke_cap(cap_node_t* cn, cap_t cap) __attribute__((noreturn));
