.section .payload
#ifdef PAYLOAD
incbin PAYLOAD
#else
1:
    j   1b
#endif

