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
#include "lock.h"

typedef struct uart {
        unsigned long long txdata;
} UART;

UART *UART0 = (UART*)(0x10000000);

int _fstat(int file, struct stat* st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(const char* name, int flags, int mode)
{
    return -1;
}
int _write(int file, char* c, int len)
{
    static Lock l;
    acquire_lock(&l);
    for (int i = 0; i < len; ++i) {
        while(UART0->txdata < 0);
        UART0->txdata = c[i];
    }
    release_lock(&l);
    return len;
}

int _read(int file, char* c, int len)
{
    return -1;
}

char* _sbrk(int r)
{
    extern char _heap; /* Defined by the linker */
    extern char _eheap; /* Defined by the linker */
    static Lock l;
    acquire_lock(&l);

    static char* heap_ptr = &_heap;
    if (heap_ptr + r <= &_eheap) {
        char* base = heap_ptr;
        heap_ptr += r;
    release_lock(&l);
        return base;
    }
    release_lock(&l);
    return (char*)-1;
}
