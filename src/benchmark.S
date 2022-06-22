# See LICENSE file for copyright and license details.

.globl benchmark_code

benchmark_code:
        // Load global pointer
        .option push
        .option norelax
        la      gp,__global_pointer$
        .option pop
        mv      a0,zero // Zero register
here:
1:      addi    a0,a0,1
        j       1b
