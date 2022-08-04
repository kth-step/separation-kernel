// See LICENSE file for copyright and license details.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cap.h"
#include "config.h"
#include "lock.h"
#include "platform.h"
#include "proc.h"

void sched(void);
void sched_start(void);

bool sched_update(cap_node_t* cn, uint64_t hartid, uint64_t begin, uint64_t end,
                  uint64_t depth_expected, uint64_t pid_desired, uint64_t depth_desired);
