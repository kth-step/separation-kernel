# Dummy user code 
.globl user_code
user_code:
        # UART address
        li s0, 0x10000000

        # ptr to hello world
1:      la s1, hw_begin
        la s2, hw_end
        # Write Hello, world
2:      lb s3, 0(s1)
        sb s3, 0(s0)
        addi s1,s1,1
        bne s1,s2,2b
        # Restart
        ecall
        j 1b

.section .data
hw_begin:
.string "Hello, world\n"
hw_end:
