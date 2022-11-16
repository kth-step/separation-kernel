// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

#include "cap_node.h"
#include "proc.h"

/* Without capabilities */
proc_t* syscall_get_pid(proc_t* current);
proc_t* syscall_get_reg(proc_t* current, uint64_t regnr);
proc_t* syscall_set_reg(proc_t* current, uint64_t regnr, uint64_t val);
proc_t* syscall_yield(proc_t* current);
/* Basic capabilities */
proc_t* syscall_read_cap(proc_t* current, uint64_t cidx);
proc_t* syscall_move_cap(proc_t* current, uint64_t src_cidx, uint64_t dest_cidx);
proc_t* syscall_delete_cap(proc_t* current, uint64_t cidx);
proc_t* syscall_revoke_cap(proc_t* current, uint64_t cidx);
proc_t* syscall_derive_cap(proc_t* current, uint64_t src_cidx, uint64_t dest_cidx, uint64_t word0, uint64_t word1);
/* Superviser capabilities functions */
proc_t* syscall_sup_suspend(proc_t* current, uint64_t cidx, uint64_t pid);
proc_t* syscall_sup_resume(proc_t* current, uint64_t cidx, uint64_t pid);
proc_t* syscall_sup_get_state(proc_t* current, uint64_t cidx, uint64_t pid);
proc_t* syscall_sup_get_reg(proc_t* current, uint64_t cidx, uint64_t pid, uint64_t reg);
proc_t* syscall_sup_set_reg(proc_t* current, uint64_t cidx, uint64_t pid, uint64_t reg, uint64_t val);
proc_t* syscall_sup_read_cap(proc_t* current, uint64_t cidx, uint64_t pid, uint64_t read_cidx);
proc_t* syscall_sup_move_cap(proc_t* current, uint64_t cidx, uint64_t pid, uint64_t take, uint64_t src_cidx,
                             uint64_t dest_cidx);
