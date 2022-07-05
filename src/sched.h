// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "config.h"
#include "proc.h"
#include "timer.h"

#if TIME_SLOT_LOANING != 0
        typedef struct time_slot_instance TimeSlotInstance;
        typedef struct time_slot_instance_root TimeSlotInstanceRoot;
        
        /**
         * For each time slot of the schedule (i.e. for each hart and each quantum) there is a TimeSlotInstanceRoot, indicating 
         * what processes is scheduled. If the process set to run loans its time to another processes a TimeSlotInstance
         * will be pointed to be the root, containing the pid of the new process being scheduled in this slot.
         * If this process in turn loans the time slot another TimeSlotInstance will be pointed to by head of the root. 
         * This new TimeSlotInstance will in turn point to the previous TimeSlotInstance with its "loaner" pointer.
         * 
         * Consequently a loaner pointer that is NULL means that the TimeSlotInstance loans directly from a root, and a head
         * pointer being NULL means that there is no process being loaned the time slot.
         */
        struct time_slot_instance {
                uint8_t pid;
                /**
                 * The TimeSlotInstance that loans its execution time to this TimeSlotInstance.
                 * If NULL this TimeSlotInstance got its time directly from the schedule, i.e it loans from a TimeSlotInstanceRoot.
                 */
                TimeSlotInstance * loaner;
        };
        
        struct time_slot_instance_root {
                uint8_t * pidp;
                /**
                 * The TimeSlotInstance that currently is set to exeucte in this time slot.
                 * If NULL, this TimeSlotInstanceRoot is not loaning its time to any other instance and the pid pointed to by pidp
                 * is the the processes that will run.
                 */
                TimeSlotInstance * head;
        };
#endif

void Sched(void);

bool SchedUpdate(uint8_t begin, uint8_t end, uint8_t hartid, uint16_t expected,
                 uint16_t desired, Cap * c);
static inline void SchedUpdatePidTsid(uint8_t begin, uint8_t end,
                                      uint8_t hartid, uint8_t pid_expected,
                                      uint8_t tsid_expected,
                                      uint8_t pid_desired,
                                      uint8_t tsid_desired, 
                                      Cap * c);

void SchedUpdatePidTsid(uint8_t begin, uint8_t end, uint8_t hartid,
                        uint8_t pid_expected, uint8_t tsid_expected,
                        uint8_t pid_desired, uint8_t tsid_desired, 
                        Cap * c) {
        uint16_t expected = (tsid_expected << 8) | pid_expected;
        uint16_t desired = (tsid_desired << 8) | pid_desired;
        SchedUpdate(begin, end, hartid, expected, desired, c);
}

void InitSched();

#if TIME_SLOT_LOANING != 0
        void InitTimeSlotInstanceRoots();
#endif