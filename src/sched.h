// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "config.h"
#include "proc.h"
#include "timer.h"

/* These objects are used keep track of the parents of time slice objects. 
   This is needed because on delete the parent is supposed to get the time back.
   If the time slice object is a root time slice then the time it represents is simply made unavailable on delete.

   On delete of a time slice cap we use its tsid to look up the ts_id_status object. 
   Here we follow the parent_tsid to another ts_id_status object and check if it's actvive, and if its timestamp is older than
   that of the ts_id_object of the time slice we are deleting. If the timestamp is older and it is active, it is still our parent
   and we return our time to it.

   These values are to be updated when holding the schedule lock (so in functions SchedDelete, SchedUpdate, SchedRevoke etc).
 */
typedef struct ts_id_status {
        /* The tsid for the parent Time slice used to derive the time slice this object represents. 
           Not applicable for the root time slices (those with tsid 0).
           This value is updated for the child on derive. */
        uint8_t parent_tsid;
        /* The pid associated with the time slice this object respresents.
           This value is updated on interprocess move and for the child on derive. */
        uint8_t pid;
        /* Whether the time slice this object represents is avtice or not (if it's inactive it has been deleted). 
           This value is updated on delete (set to inactive), and for the child on derive (set to active) and revoke (set to inactive).*/
        bool tsid_active;
        /* A value used to order when various time slice objects were created.
           This is needed to avoid the ABA problem. 

           Example: if A is a parent to X and A is deleted, X is now a child of A:s parent: B. If B creates a new time slice with the same tsid as A, 
           X can't tell that this new time slice is not its parent. Thus, we need to utilize timestamps to show that this new time slice is in fact not a parent of X.
           While it's not possible for B to create the new time slice before deleting all its children (including X), it possible for the creation of the new time slice to happen
           after deleting X but before updating the schedule and these ts_id_status objects, creating a situation where the entire operation of creating the new time slice
           finishes earlier than the entire operation of deleting X.

           In this situation when we delete X we will look at the parent_tsid, check that what used to be A is no longer our parent, and follow that object's parent_tsid to find B
           (which we give the time of X).
           If the time slice replacing A is not directly a child of B, all ancestors of this object until B must be newer than X and thus we can just follow the chain all the way
           up to B. 

           Furthermore, B is necessarily the direct parent of X in this situation (we can't have this scenario but with time slices between the original A and B), 
           meaning it's correct to give B the time if X is deleted. This is because all X's ancestors up until the first one shared with A's replcement must 
           have been deleted, otherwsie it wouldn't have been possible to create A's replacement.

           This value is updated for the child on derive. */
        uint64_t creation_timestamp;
} TsIdStatus;

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

bool SchedDelete(uint64_t begin, uint64_t end, uint8_t hartid, uint16_t expected);

bool SchedDeleteAssumeNoPreemption(uint64_t begin, uint64_t end, uint8_t hartid, uint16_t expected);

void InitSched();

#if TIME_SLOT_LOANING != 0
        void InitTimeSlotInstanceRoots();
#elif TIME_SLOT_LOANING_SIMPLE != 0
        bool SchedTryLoanTime(uint64_t pid);
        void SchedReturnTime();
#endif