// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

#include "cap_node.h"
uint64_t syscall_unimplemented(void);
uint64_t syscall_read_cap(uint64_t cidx);
uint64_t syscall_move_cap(uint64_t src_cidx, uint64_t dest_cidx);
uint64_t syscall_delete_cap(uint64_t cidx);
uint64_t syscall_revoke_cap(uint64_t cidx);
uint64_t syscall_derive_cap(uint64_t src_cidx, uint64_t dest_cidx, uint64_t word0, uint64_t word1);

uint64_t syscall_get_pid(void);
uint64_t syscall_read_reg(uint64_t regnr);
uint64_t syscall_write_reg(uint64_t regnr, uint64_t val);
void syscall_yield(void);
