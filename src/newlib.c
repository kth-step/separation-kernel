/**
 * Copyright 2020, Saab AB
 *
 * This software may be distributed and modified according to
 * the terms of the GNU General Public License version 2.
 * Note that NO WARRANTY is provided.
 * See "LICENSE.GPLv2" for details.
 */
#include <string.h>
#include <sys/stat.h>

typedef struct uart {
        int txdata;
        int rxdata;
        int txctrl;
        int rxctrl;
        int ie;
        int ip;
        int div;
} UART;

extern volatile UART uart0;

int _fstat(int file, struct stat *st) {
        st->st_mode = S_IFCHR;
        return 0;
}

int _isatty(int file) {
        return 1;
}

int _lseek(int file, int ptr, int dir) {
        return 0;
}

int _open(const char *name, int flags, int mode) {
        return -1;
}
int _write(int file, char *c, int len) {
        for (int i = 0; i < len; ++i) {
                if (c[i] == '\n') {
                        while (uart0.txdata < 0)
                                ;
                        uart0.txdata = '\r';
                }
                while (uart0.txdata < 0)
                        ;
                uart0.txdata = c[i];
        }
        return len;
}

int _read(int file, char *c, int len) {
        return -1;
}

char *_sbrk(int r) {
        extern char _heap;  /* Defined by the linker */
        extern char _eheap; /* Defined by the linker */

        static char *heap_ptr = &_heap;
        if (heap_ptr + r <= &_eheap) {
                char *base = heap_ptr;
                heap_ptr += r;
                return base;
        }
        return (char *)-1;
}
