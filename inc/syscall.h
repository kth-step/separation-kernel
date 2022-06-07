// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "cap.h"
#include "proc.h"

/* Get process ID */

void SyscallHandler(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7);
