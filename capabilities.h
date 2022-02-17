// See LICENSE file for copyright and license details.

#pragma once
#include "types.h"

// Capability {{{
typedef union capability {
        uintptr_t type:4;
        struct {
                uintptr_t type:4;
                uintptr_t cfg:4;
                uintptr_t addr:56;
                uintptr_t addr_tor:56;
        } pmp_entry;
        struct {
                uintptr_t type:4;
                uintptr_t rwx:4;
                uintptr_t begin:56;
                uintptr_t end:56;
        } mem_slice;
        struct {
                uintptr_t type:4;
                uintptr_t begin:8;
                uintptr_t end:8;
                uintptr_t fuel:8;
        } time_slice;
} Capability;
_Static_assert(sizeof(Capability) == 16, "Capability size error");
// }}}

