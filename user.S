# Dummy user code 
.globl user_code
user_code:
        ecall
        addi    a7,a7,1
        j user_code
