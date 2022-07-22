// See LICENSE file for copyright and license details.
#include "proc.h"

#include <stddef.h>

#include "cap.h"
#include "cap_util.h"
#include "config.h"
#include "sched.h"
#include "stack.h"

/** Initial stack offset.
 * The initialization of a process is a bit awkward, we basically
 * start in the middle of a function call, for this we have this
 * stack offset which should be safe. It must fit all registers saved
 * to the stack in switch.S and entry.S.
 */
#define INIT_STACK_OFFSET (STACK_SIZE / 8 - 32)

/* Temporary. */
extern void user_code();

/* Benchmarking */
#if IPC_BENCHMARK != 0
        extern void ipc_benchmark();
        extern Proc *volatile channels[N_CHANNELS];
#endif
#if SCHEDULE_BENCHMARK != 0
        extern void benchmark_code();
#endif
#if CRYPTO_APP != 0
        extern void crypto_decrypt_code();
        extern void cypher_provider_code();
        extern void plaintext_consumer_code();
#endif

/* Defined in proc.h */
Proc processes[N_PROC];

static void proc_init_boot_proc(Proc *boot);

#if IPC_BENCHMARK != 0
void ProcIpcInit() {
        uint64_t dummy_msg[4];

        processes[0].pc = (uintptr_t)ipc_benchmark;
        /* We just make all our receivers and senders children of the root channel for simplicity */
        cap_set(&processes[0].cap_table[3 + N_CORES + N_PROC], cap_serialize_sender(cap_mk_sender(1)));
        CapAppend(&processes[0].cap_table[3 + N_CORES + N_PROC], &processes[0].cap_table[2]);
        for (int i = 1; i < N_PROC; i++) {
                Proc * p = &processes[i];
                p->args[1] = 2; // Where to place caps
                p->args[2] = 1; // How many caps
                p->args[3] = (uintptr_t)dummy_msg;
                cap_set(&processes[i].cap_table[0], cap_serialize_receiver(cap_mk_receiver(i)));
                CapAppend(&processes[i].cap_table[0], &processes[0].cap_table[2]);
                cap_set(&processes[i].cap_table[1], cap_serialize_sender(cap_mk_sender(i+1)));
                CapAppend(&processes[i].cap_table[1], &processes[0].cap_table[2]);

                processes[i].pc = (uintptr_t)ipc_benchmark;

                processes[i].listen_channel = i;
                channels[i] = &processes[i];
                processes[i].state = PROC_WAITING;
        }
}
#endif

#if CRYPTO_APP != 0
void ProcCryptoAppInit() {
        if (N_PROC < 3) {
                return;
        }
        // TODO: give processes 1 and 2 memory capabilities; right now it only works since they aren't enforced.
        processes[0].pc = (uintptr_t)crypto_decrypt_code;
        processes[1].pc = (uintptr_t)cypher_provider_code;
        processes[2].pc = (uintptr_t)plaintext_consumer_code;
}
#endif

/* Initializes one process. */
void ProcReset(int pid) {
        /* Get the PCB */
        Proc *proc = &processes[pid];
        /* Set the process id to */
        proc->pid = pid;
        /* Set the process's kernel stack. */
        proc->ksp = &proc_stack[pid][STACK_SIZE / 8];
        for (int i = 0; i < STACK_SIZE / 8; i++)
                proc_stack[pid][i] = 0;
        /* Zero the capability table. */
        proc->cap_table = cap_tables[pid];
        for (int i = 0; i < N_CAPS; ++i) {
                Cap *cap = &cap_tables[pid][i];
                if (!cap_is_deleted(cap)) {
                        CapRevoke(cap);
                        CapDelete(cap);
                }
        }
        proc->pc = 0;
        proc->listen_channel = -1;
        /* Set process to HALTED. */
        //proc->state = PROC_HALTED;

        // TODO: resume relevant processes through function calls instead of setting all processes to be ready. 
        proc->state = PROC_SUSPENDED;
}

void proc_init_memory(Cap *pmp, Cap *memory) {
        uint64_t begin = USER_MEMORY_BEGIN;
        uint64_t end = USER_MEMORY_END;
        uint64_t pmp_length = BOOT_PMP_LENGTH;
        uint64_t pmp_addr = begin | ((pmp_length - 1) >> 1);
        CapPmpEntry pe = cap_mk_pmp_entry(pmp_addr, 5);
        CapMemorySlice ms = cap_mk_memory_slice(begin, end, 7);
        cap_set(pmp, cap_serialize_pmp_entry(pe));
        cap_set(memory, cap_serialize_memory_slice(ms));
        Cap *sentinel = CapInitSentinel();
        CapAppend(memory, sentinel);
        CapAppend(pmp, sentinel);
}

void proc_init_channels(Cap *channel) {
        uint16_t begin = 0;
        uint16_t end = N_CHANNELS - 1;
        CapChannels ch = cap_mk_channels(begin, end);
        cap_set(channel, cap_serialize_channels(ch));
        Cap *sentinel = CapInitSentinel();
        CapAppend(channel, sentinel);
}

void proc_init_time(Cap time[N_CORES]) {
        Cap *sentinel;
        uint16_t begin, end;
        uint8_t tsid, fuel;
        CapTimeSlice ts;
        for (int hartid = 0; hartid < N_CORES; hartid++) {
                sentinel = CapInitSentinel();
                begin = 0;
                end = N_QUANTUM;
                tsid = 0;
                fuel = 255;
                ts = cap_mk_time_slice(hartid, begin, end, tsid, fuel);
                cap_set(&time[hartid], cap_serialize_time_slice(ts));
                CapAppend(&time[hartid], sentinel);
        }
}

void proc_init_supervisor(Cap cap_sups[N_PROC]) {
        Cap *sentinel = CapInitSentinel();
        for (int pid = 0; pid < N_PROC; pid++) {
                CapSupervisor sup = cap_mk_supervisor(pid);
                cap_set(&cap_sups[pid], cap_serialize_supervisor(sup));
                CapAppend(&cap_sups[pid], sentinel);
        }
}

static void proc_init_boot_proc(Proc *boot) {
        Cap *cap_table = boot->cap_table;
        proc_init_memory(&cap_table[0], &cap_table[1]);
        proc_init_channels(&cap_table[2]);
        proc_init_time(&cap_table[3]);
        proc_init_supervisor(&cap_table[3 + N_CORES]);
        /* Set the initial PC. */
        // boot->pc = (uintptr_t)(pe_begin << 2);
        boot->pc = (uintptr_t)user_code;  // Temporary code.

        /* Set boot process to running. */
        boot->state = PROC_SUSPENDED;
}

/* Defined in proc.h */
void ProcInitProcesses(void) {
        /* Initialize processes. */
        for (int i = 0; i < N_PROC; i++)
                ProcReset(i);
        /*** Boot process ***/
        proc_init_boot_proc(&processes[0]);
        #if SCHEDULE_BENCHMARK != 0
                processes[0].pc = (uintptr_t)benchmark_code;
        #endif
        #if IPC_BENCHMARK != 0 
                ProcIpcInit();
        #endif
        #if CRYPTO_APP != 0
                ProcCryptoAppInit();
                InitSched();
        #endif
        #if TIME_SLOT_LOANING != 0
                InitTimeSlotInstanceRoots();
        #endif
}

void ProcHalt(Proc *proc) {
        proc->halt = true;
        __sync_bool_compare_and_swap(&proc->state, PROC_SUSPENDED, PROC_HALTED);
}
