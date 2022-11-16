.section .payload, "ax"
payload:
#ifndef PAYLOAD
1:      j       1b
#else
        .incbin PAYLOAD
#endif
