# See LICENSE file for copyright and license details.

.globl ipc_benchmark
#include "config.h"

#define USER_DATA_SIZE 4096

ipc_benchmark:
        // Load global pointer
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop

        // Do syscall, get_pid
        li      a0,0
        li      a1,0
        li      a7,1
        ecall                           // Fetch process ID to a0                    

        // Prepare stack pointer
        la      a1,user_data           
        li      a2,USER_DATA_SIZE      
        mul     t0,a0,a2
        add     a1,a1,t0
        add     a2,a1,a2
        mv      sp,a2

        // Zero registers
        mv      t0,zero
        mv      a1,zero
        mv      a2,zero
        mv      a7,zero

        // Call ipc_benchmark_main with pid as the first and only argument (a0)
        call    ipc_benchmark_main
1:
        j       1b

.section .bss.user_data
// This is data for all processes
user_data: .fill (USER_DATA_SIZE * N_PROC)

