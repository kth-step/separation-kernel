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

bool SchedUpdate(uint64_t begin, uint64_t end, uint8_t hartid, uint16_t expected,
                 uint16_t desired, Cap * c);

bool SchedRevoke(uint64_t begin, uint64_t end, uint8_t hartid,
                 uint16_t desired, Cap * c);

bool SchedDelete(uint64_t begin, uint64_t end, uint8_t hartid, uint16_t expected,
                 uint16_t desired);

bool SchedDeleteAssumeNoPreemption(uint64_t begin, uint64_t end, uint8_t hartid, uint16_t expected,
                 uint16_t desired);

void InitSched();

#if TIME_SLOT_LOANING != 0
        void InitTimeSlotInstanceRoots();
#endif