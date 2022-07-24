#pragma once
#include "proc.h"

bool ipc_subscribe(proc_t* proc, uint64_t channel);
void ipc_unsubscribe(uint64_t channel);
proc_t* ipc_get_subscriber(uint64_t channel);
