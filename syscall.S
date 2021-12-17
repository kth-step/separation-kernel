# See LICENSE file for copyright and license details.

.globl AsmSyscallTrampoline

.section .text.syscall

# System Call Trampoline {{{
.option push
.option norvc
AsmSyscallTrampoline:
        j       __dummy                 //  0:
        j       __dummy                 //  1:
        j       __dummy                 //  2:
        j       __dummy                 //  3:
        j       __dummy                 //  4:
        j       __dummy                 //  5:
        j       __dummy                 //  6:
        j       __dummy                 //  7:
        j       __dummy                 //  8:
        j       __dummy                 //  9:
        j       __dummy                 // 10:
        j       __dummy                 // 11:
        j       __dummy                 // 12:
        j       __dummy                 // 13:
        j       __dummy                 // 14:
        j       __dummy                 // 15:
.option pop
# }}}

# Dummy {{{
__dummy:
        ret
# }}}
