.globl payload
.section .payload, "a"
#ifndef PAYLOAD
1:      j       1b
#else
        .incbin PAYLOAD
#endif
