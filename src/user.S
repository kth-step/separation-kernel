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
        la      ra,user_trampoline
        slli    t0,a0,2
        add     ra,ra,t0
        jalr    ra,(ra)
1:      j       1b

user_trampoline:
.option push
.option norvc
        j       main_supervisor
        j       main_uart
        j       main_app1
        j       main_app2
        j       main_crypt
.option pop

.section .bss.user_data
// This is data for all processes
user_data: .fill (USER_DATA_SIZE * N_PROC)

