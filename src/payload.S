// See LICENSE file for copyright and license details.
.section .payload
#ifdef PAYLOAD
incbin PAYLOAD
#else
1:
    j   1b
#endif

