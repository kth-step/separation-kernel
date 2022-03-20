// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

typedef enum cap_type {
        CAP_INVALID, CAP_PMP_ENTRY, CAP_MEMORY_SLICE, CAP_TIME_SLICE
} CapType;

// Capability {{{
typedef struct {
        CapType type:8;
        uintptr_t addr:56;      // pmpaddr field (lower bound if TOR)
        uintptr_t cfg:8;        // pmpcfg field
        uintptr_t addr_tor:56;  // second pmpaddr field (upper bound if TOR)
} PmpEntry;
typedef struct {
        CapType type:8;
        uintptr_t begin:56;     // Start of memory slice.
        uintptr_t rwx:8;        // Read, write, execute permissions.
        uintptr_t end:56;       // End of memory slice.
} MemorySlice;
typedef struct {
        CapType type:8;
        uintptr_t cid:8;        // Core to run on.
        uintptr_t tsid:8;       // TimeSlice ID.
        uintptr_t fuel:8;       // Max number of children.
        uintptr_t begin:8;      // Start of time slice.
        uintptr_t end:8;        // End of time slice.
} TimeSlice;
typedef union capability {
        CapType type:8;
        PmpEntry pmp_entry;
        MemorySlice memory_slice;
        TimeSlice time_slice;
} Capability;
_Static_assert(sizeof(Capability) == 16, "Capability size error");
// }}}

