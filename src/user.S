# See LICENSE file for copyright and license details.
# Dummy user code 
.globl user_code
.extern main
#include "config.h"

#define USER_DATA_SIZE 4096

user_code:
        // Load global pointer
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop

        // Do syscall, get_pid
        li      a7,0
        ecall                           // Fetch process ID to a0                    

        // Prepare user data
        la      a1,user_data            // a2=user_data
        li      a2,USER_DATA_SIZE       // t0=USER_DATA_SIZE
        mul     t0,a0,a2
        add     a1,a1,t0
        add     a2,a1,a2
        mv      sp,a2
        // Zero register
        mv      t0,zero

        // Call main with (pid, user data start, user data end)
        mv      t3,a0
        call    user_main
        j       user_code

.section .bss.user_data
// This is data for all processes
user_data: .fill (USER_DATA_SIZE * N_PROC)

