.globl root_payload

.section .data.payload
root_payload:
#ifdef PAYLOAD
        .incbin PAYLOAD
#else
1:      j       1b
#endif
