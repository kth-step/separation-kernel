// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "config.h"
#include "proc.h"
#include "timer.h"

void Sched(void);

void SchedUpdate(uint8_t begin, uint8_t end, uint8_t hartid, uint16_t expected,
                 uint16_t desired);
static inline void SchedUpdatePidTsid(uint8_t begin, uint8_t end,
                                      uint8_t hartid, uint8_t pid_expected,
                                      uint8_t tsid_expected,
                                      uint8_t pid_desired,
                                      uint8_t tsid_desired);

void SchedUpdatePidTsid(uint8_t begin, uint8_t end, uint8_t hartid,
                        uint8_t pid_expected, uint8_t tsid_expected,
                        uint8_t pid_desired, uint8_t tsid_desired) {
        uint16_t expected = (tsid_expected << 8) | pid_expected;
        uint16_t desired = (tsid_desired << 8) | pid_desired;
        SchedUpdate(begin, end, hartid, expected, desired);
}

void InitSched();