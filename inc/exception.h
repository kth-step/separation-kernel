// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

#include "proc.h"
proc_t* exception_handler(proc_t* proc, uint64_t mcause, uint64_t mtval, uint64_t mepc);
