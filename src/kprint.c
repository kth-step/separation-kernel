#include "kprint.h"

#include "lock.h"
#include "platform.h"
#include "snprintf.h"

int kprintf(const char* format, ...)
{
        static lock_t lock = INIT_LOCK;
        va_list args;
        char buf[256];
        char *b = buf;
        int i;

        lock_acquire(&lock);
        va_start(args, format);
        i = vsnprintf(buf, 256, format, args);
        while (*b != '\0') {
                uart_putchar(*b++);
        }
        lock_release(&lock);
        return i;
}

