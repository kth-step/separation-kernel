#include "ipc.h"

proc_t* subscribers[N_CHANNELS];

bool ipc_subscribe(proc_t* proc, uint64_t channel)
{
        return __sync_bool_compare_and_swap(&subscribers[channel], NULL, proc);
}

void ipc_unsubscribe(uint64_t channel)
{
        subscribers[channel] = NULL;
}

proc_t* ipc_get_subscriber(uint64_t channel)
{
        return subscribers[channel];
}
