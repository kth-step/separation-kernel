# See LICENSE file for copyright and license details.

.globl crypto_decrypt_code
.globl cypher_provider_code
.globl plaintext_consumer_code

#include "../src/config.h"

#define USER_DATA_SIZE 4096

crypto_decrypt_code:
        mv      t4,zero
        j       1f
cypher_provider_code:
        addi    t4,zero,1
        j       1f
plaintext_consumer_code:
        addi    t4,zero,2

1:
        // Load global pointer
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop

        // Do syscall, get_pid
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

        // Prepare user data
        /* Load some constants we will use */
        li      t0,2
        li      t1,8
        li      t2,CRYPTO_MEMORY_SIZE

        /* Load the beginning of the memory area */
        la      a0,crypto_memory_area
        /* Get the end of the memory area */
        add     t3,a0,t2

        /* Load end - 8 bytes into arguments */
        sub     a4,t3,t1
        sub     a5,t3,t1

        /* Get middle of memory area */
        div     t3,t2,t0 
        add     t2,a0,t3
        /* Add middle of memory area as argument, and middle - 8 bytes as arguments */
        addi    a3,t2,0
        sub     a1,t2,t1
        sub     a2,t2,t1

        // Zero registers
        mv      t0,zero
        mv      t1,zero
        mv      t2,zero
        mv      t3,zero
        mv      a7,zero

        addi    t0,zero,1
        beq     t4,t0,2f
        bnez    t4,3f

        call    crypto_decrypt_main
        j       crypto_decrypt_code

2:
        // Zero additional registers
        mv      t0,zero
        mv      a3,zero
        mv      a4,zero
        mv      a5,zero
        call    cypher_provider_main
        j       cypher_provider_code

3:
        // Rearrange arguments
        mv      a0,a3
        mv      a1,a4
        mv      a2,a5

        // Zero additional registers
        mv      t0,zero
        mv      a3,zero
        mv      a4,zero
        mv      a5,zero
        j       plaintext_consumer_main
        j       plaintext_consumer_code

.section .bss.user_data
// This is data for all processes
user_data: .fill (USER_DATA_SIZE * N_PROC)